#include <pebble.h>

#include "wordclock.h"

#define SETTINGS_KEY 1
#define TEMPERATURE_KEY 2
#define WEATHER_INTERVAL_MIN 30
#define INFO_LINE_WIDTH 2

typedef struct {
  GColor background;
  GColor text;
  GColor highlight;
  bool show_date;
  bool show_temp;
  bool fahrenheit;
  // Neue Felder nur hinten anhängen: ältere gespeicherte Einstellungen sind kürzer,
  // die fehlenden Felder behalten dann ihren Default.
  bool show_es_ist;
  bool info_grid_style;
  // Farben der Infozeile im Textmodus; ohne eigene Farben invertiert zum Raster.
  bool info_custom_colors;
  GColor info_background;
  GColor info_text;
  GColor info_line;
} Settings;

static Settings s_settings;
static Window *s_window;
static Layer *s_grid_layer;
static GFont s_font;
static int s_font_height;
static bool s_on[WC_ROWS][WC_COLS];
static bool s_has_temp;
static int s_temp;

static void default_settings(void) {
  s_settings.background = GColorWhite;
  // Auf Schwarzweiß-Displays gibt es kein Grau; dort sieht man nur die aktiven Wörter.
  s_settings.text = PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite);
  s_settings.highlight = GColorBlack;
  s_settings.show_date = false;
  s_settings.show_temp = false;
  s_settings.fahrenheit = false;
  s_settings.show_es_ist = false;
  s_settings.info_grid_style = true;
  s_settings.info_custom_colors = false;
  s_settings.info_background = GColorBlack;
  s_settings.info_text = GColorWhite;
  s_settings.info_line = PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite);
}

static void load_settings(void) {
  default_settings();
  persist_read_data(SETTINGS_KEY, &s_settings, sizeof(s_settings));
  if (persist_exists(TEMPERATURE_KEY)) {
    s_temp = persist_read_int(TEMPERATURE_KEY);
    s_has_temp = true;
  }
}

static bool info_row_visible(void) {
  return s_settings.show_date || (s_settings.show_temp && s_has_temp);
}

static void request_weather(void) {
  if (!s_settings.show_temp) {
    return;
  }
  DictionaryIterator *out;
  if (app_message_outbox_begin(&out) != APP_MSG_OK) {
    return;
  }
  dict_write_uint8(out, MESSAGE_KEY_RequestWeather, 1);
  dict_write_uint8(out, MESSAGE_KEY_Fahrenheit, s_settings.fahrenheit);
  app_message_outbox_send();
}

typedef struct {
  GRect origin;   // erste Zelle der Zeile
  int cell_w;
  int cell_h;
} GridRow;

static void draw_cell(GContext *ctx, const GridRow *row, int col, const char *text, bool on) {
  graphics_context_set_text_color(ctx, on ? s_settings.highlight : s_settings.text);
  GRect cell = GRect(row->origin.origin.x + col * row->cell_w, row->origin.origin.y,
                     row->cell_w, row->cell_h);
  graphics_draw_text(ctx, text, s_font, cell,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

// Schreibt die Zeichen von `text` ab Spalte `col` als leuchtende Zellen; "°" wird angehängt,
// wenn `degree` gesetzt ist. Gibt die nächste freie Spalte zurück.
static int put_cells(char cells[WC_COLS][4], bool on[WC_COLS], int col, const char *text,
                     bool degree) {
  for (const char *p = text; *p && col < WC_COLS; p++, col++) {
    cells[col][0] = *p;
    cells[col][1] = '\0';
    on[col] = true;
  }
  if (degree && col < WC_COLS) {
    strcpy(cells[col], "°");
    on[col] = true;
    col++;
  }
  return col;
}

// Datum und Temperatur im Wortuhr-Stil: ein Zeichen pro Zelle, dazwischen
// ausgegraute Füllbuchstaben wie im restlichen Raster.
static void draw_info_row_grid(GContext *ctx, const GridRow *row,
                               const char *weekday, const char *date, const char *temp) {
  static const char filler[] = "PQXYJKVZWRM";
  char cells[WC_COLS][4];
  bool on[WC_COLS] = {false};
  for (int c = 0; c < WC_COLS; c++) {
    cells[c][0] = filler[c];
    cells[c][1] = '\0';
  }

  int date_len = strlen(date);
  int temp_len = temp[0] ? (int)strlen(temp) + 1 : 0;   // + Gradzeichen
  // Wochentag nur, wenn mit je einer Füllzelle Abstand alles in die Zeile passt.
  bool with_weekday = date_len &&
      2 + 1 + date_len + (temp_len ? 1 + temp_len : 0) <= WC_COLS;
  int left_len = date_len ? date_len + (with_weekday ? 3 : 0) : 0;

  if (left_len && temp_len) {
    int col = 0;
    if (with_weekday) {
      col = put_cells(cells, on, col, weekday, false) + 1;
    }
    put_cells(cells, on, col, date, false);
    put_cells(cells, on, WC_COLS - temp_len, temp, true);
  } else if (left_len) {
    int col = (WC_COLS - left_len) / 2;
    if (with_weekday) {
      col = put_cells(cells, on, col, weekday, false) + 1;
    }
    put_cells(cells, on, col, date, false);
  } else {
    put_cells(cells, on, (WC_COLS - temp_len) / 2, temp, true);
  }

  for (int c = 0; c < WC_COLS; c++) {
    draw_cell(ctx, row, c, cells[c], on[c]);
  }
}

// `bar` ist der Bereich vom oberen Rand bis zur Unterkante der Infozeile.
static void draw_info_row(GContext *ctx, const GridRow *row, GRect bar) {
  static const char * const weekdays[] = {"SO", "MO", "DI", "MI", "DO", "FR", "SA"};
  const char *weekday = "";
  char day_month[24] = "";
  char temp[8] = "";

  if (s_settings.show_date) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    weekday = weekdays[t->tm_wday];
    snprintf(day_month, sizeof(day_month), "%d.%d", t->tm_mday, t->tm_mon + 1);
  }
  if (s_settings.show_temp && s_has_temp) {
    snprintf(temp, sizeof(temp), "%d", s_temp);
  }

  if (s_settings.info_grid_style) {
    draw_info_row_grid(ctx, row, weekday, day_month, temp);
    return;
  }

  // Textmodus: eigener Balken mit Trennlinie, damit sich die Zeile vom Raster abhebt.
  GColor bar_background = s_settings.highlight;
  GColor bar_text = s_settings.background;
  GColor bar_line = s_settings.text;
  if (s_settings.info_custom_colors) {
    bar_background = s_settings.info_background;
    bar_text = s_settings.info_text;
    bar_line = s_settings.info_line;
  }
  graphics_context_set_fill_color(ctx, bar_background);
  graphics_fill_rect(ctx, bar, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, bar_line);
  graphics_fill_rect(ctx, GRect(bar.origin.x, bar.origin.y + bar.size.h - INFO_LINE_WIDTH,
                                bar.size.w, INFO_LINE_WIDTH), 0, GCornerNone);

  // Text innerhalb der äußeren Buchstabenspalten, damit er bündig mit dem Raster ist.
  char date[32] = "";
  if (day_month[0]) {
    snprintf(date, sizeof(date), "%s %s.", weekday, day_month);
  }
  if (temp[0]) {
    strcat(temp, "°");
  }
  int inset = row->cell_w / 4;
  // Nach oben schieben, damit der Text mittig im Balken (inkl. oberem Rand) über der
  // Linie steht statt auf ihr.
  int lift = INFO_LINE_WIDTH + (bar.size.h - row->cell_h) / 2;
  GRect text_row = GRect(row->origin.origin.x + inset, row->origin.origin.y - lift,
                         row->cell_w * WC_COLS - 2 * inset, row->cell_h);

  graphics_context_set_text_color(ctx, bar_text);
  if (date[0] && temp[0]) {
    graphics_draw_text(ctx, date, s_font, text_row, GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, temp, s_font, text_row, GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentRight, NULL);
  } else {
    graphics_draw_text(ctx, date[0] ? date : temp, s_font, text_row,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  }
}

static void grid_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_fill_color(ctx, s_settings.background);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Infozeile und "ES IST" sind optional; ohne sie verteilt sich das Raster auf die
  // volle Höhe.
  bool info = info_row_visible();
  bool es_ist = s_settings.show_es_ist;
  int rows = WC_ROWS + (info ? 1 : 0) + (es_ist ? 1 : 0);
  int cell_w = bounds.size.w / WC_COLS;
  int cell_h = bounds.size.h / rows;
  int pad_x = (bounds.size.w - cell_w * WC_COLS) / 2;
  int pad_y = (bounds.size.h - cell_h * rows) / 2;

  // graphics_draw_text setzt oberhalb der Glyphe Luft an; die ziehen wir wieder ab.
  int text_dy = (cell_h - s_font_height) / 2 - 3;

  GridRow row = { GRect(pad_x, pad_y + text_dy, 0, 0), cell_w, cell_h };

  if (info) {
    draw_info_row(ctx, &row, GRect(0, 0, bounds.size.w, pad_y + cell_h));
    row.origin.origin.y += cell_h;
  }

  if (es_ist) {
    for (int c = 0; c < WC_COLS; c++) {
      bool on = (c >= WC_ES_FROM && c <= WC_ES_TO) || (c >= WC_IST_FROM && c <= WC_IST_TO);
      draw_cell(ctx, &row, c, wc_es_ist_row[c], on);
    }
    row.origin.origin.y += cell_h;
  }

  for (int r = 0; r < WC_ROWS; r++) {
    for (int c = 0; c < WC_COLS; c++) {
      draw_cell(ctx, &row, c, wc_grid[r][c], s_on[r][c]);
    }
    row.origin.origin.y += cell_h;
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  wc_compute(tick_time->tm_hour, tick_time->tm_min, s_on);
  layer_mark_dirty(s_grid_layer);
  if (tick_time->tm_min % WEATHER_INTERVAL_MIN == 0) {
    request_weather();
  }
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *t;

  // Temperatur vom Handy
  if ((t = dict_find(iter, MESSAGE_KEY_Temperature))) {
    s_temp = t->value->int32;
    s_has_temp = true;
    persist_write_int(TEMPERATURE_KEY, s_temp);
  }

  // Einstellungen aus der Clay-Konfigurationsseite
  bool settings_changed = false;
  if ((t = dict_find(iter, MESSAGE_KEY_BackgroundColor))) {
    s_settings.background = GColorFromHEX(t->value->int32);
    settings_changed = true;
  }
  if ((t = dict_find(iter, MESSAGE_KEY_TextColor))) {
    s_settings.text = GColorFromHEX(t->value->int32);
    settings_changed = true;
  }
  if ((t = dict_find(iter, MESSAGE_KEY_HighlightColor))) {
    s_settings.highlight = GColorFromHEX(t->value->int32);
    settings_changed = true;
  }
  if ((t = dict_find(iter, MESSAGE_KEY_ShowEsIst))) {
    s_settings.show_es_ist = t->value->int32 == 1;
    settings_changed = true;
  }
  if ((t = dict_find(iter, MESSAGE_KEY_InfoGridStyle))) {
    s_settings.info_grid_style = t->value->int32 == 1;
    settings_changed = true;
  }
  if ((t = dict_find(iter, MESSAGE_KEY_InfoCustomColors))) {
    s_settings.info_custom_colors = t->value->int32 == 1;
    settings_changed = true;
  }
  if ((t = dict_find(iter, MESSAGE_KEY_InfoBackgroundColor))) {
    s_settings.info_background = GColorFromHEX(t->value->int32);
    settings_changed = true;
  }
  if ((t = dict_find(iter, MESSAGE_KEY_InfoTextColor))) {
    s_settings.info_text = GColorFromHEX(t->value->int32);
    settings_changed = true;
  }
  if ((t = dict_find(iter, MESSAGE_KEY_InfoLineColor))) {
    s_settings.info_line = GColorFromHEX(t->value->int32);
    settings_changed = true;
  }
  if ((t = dict_find(iter, MESSAGE_KEY_ShowDate))) {
    s_settings.show_date = t->value->int32 == 1;
    settings_changed = true;
  }
  bool unit_changed = false;
  if ((t = dict_find(iter, MESSAGE_KEY_Fahrenheit))) {
    bool fahrenheit = t->value->int32 == 1;
    unit_changed = fahrenheit != s_settings.fahrenheit;
    s_settings.fahrenheit = fahrenheit;
    settings_changed = true;
  }
  if ((t = dict_find(iter, MESSAGE_KEY_ShowTemp))) {
    s_settings.show_temp = t->value->int32 == 1;
    settings_changed = true;
  }

  if (settings_changed) {
    persist_write_data(SETTINGS_KEY, &s_settings, sizeof(s_settings));
    window_set_background_color(s_window, s_settings.background);
    if (unit_changed) {
      // Alter Wert ist in der falschen Einheit
      s_has_temp = false;
      persist_delete(TEMPERATURE_KEY);
    }
  }

  // Frische Temperatur holen, wenn die JS-Seite gerade gestartet ist oder
  // die Einstellungen neu gespeichert wurden.
  if (settings_changed || dict_find(iter, MESSAGE_KEY_RequestWeather)) {
    request_weather();
  }

  layer_mark_dirty(s_grid_layer);
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);

  // Auf den schmalen 144px-Displays passt die große Schrift nicht mehr ins Raster.
  if (bounds.size.w / WC_COLS >= 17) {
    s_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
    s_font_height = 24;
  } else {
    s_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    s_font_height = 18;
  }

  s_grid_layer = layer_create(bounds);
  layer_set_update_proc(s_grid_layer, grid_update_proc);
  layer_add_child(root, s_grid_layer);

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  wc_compute(t->tm_hour, t->tm_min, s_on);
}

static void window_unload(Window *window) {
  layer_destroy(s_grid_layer);
}

static void init(void) {
  load_settings();

  s_window = window_create();
  window_set_background_color(s_window, s_settings.background);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(256, 64);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
  return 0;
}
