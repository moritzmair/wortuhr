#include <pebble.h>

#include "wordclock.h"

#define SETTINGS_KEY 1
#define TEMPERATURE_KEY 2
#define WEATHER_INTERVAL_MIN 30

typedef struct {
  GColor background;
  GColor text;
  GColor highlight;
  bool show_date;
  bool show_temp;
  bool fahrenheit;
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

static void draw_info_row(GContext *ctx, GRect row) {
  static const char * const weekdays[] = {"SO", "MO", "DI", "MI", "DO", "FR", "SA"};
  char date[16] = "";
  char temp[8] = "";

  if (s_settings.show_date) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    snprintf(date, sizeof(date), "%s %d.%d.", weekdays[t->tm_wday], t->tm_mday, t->tm_mon + 1);
  }
  if (s_settings.show_temp && s_has_temp) {
    snprintf(temp, sizeof(temp), "%d°", s_temp);
  }

  graphics_context_set_text_color(ctx, s_settings.highlight);
  if (date[0] && temp[0]) {
    graphics_draw_text(ctx, date, s_font, row, GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, temp, s_font, row, GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentRight, NULL);
  } else {
    graphics_draw_text(ctx, date[0] ? date : temp, s_font, row,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  }
}

static void grid_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_fill_color(ctx, s_settings.background);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Die Infozeile ersetzt die frühere "ES IST"-Zeile; ohne sie verteilt sich das Raster
  // auf die volle Höhe.
  bool info = info_row_visible();
  int rows = WC_ROWS + (info ? 1 : 0);
  int cell_w = bounds.size.w / WC_COLS;
  int cell_h = bounds.size.h / rows;
  int pad_x = (bounds.size.w - cell_w * WC_COLS) / 2;
  int pad_y = (bounds.size.h - cell_h * rows) / 2;

  // graphics_draw_text setzt oberhalb der Glyphe Luft an; die ziehen wir wieder ab.
  int text_dy = (cell_h - s_font_height) / 2 - 3;

  if (info) {
    // Innerhalb der äußeren Buchstabenspalten, damit die Zeile bündig mit dem Raster ist.
    int inset = cell_w / 4;
    draw_info_row(ctx, GRect(pad_x + inset, pad_y + text_dy,
                             cell_w * WC_COLS - 2 * inset, cell_h));
    pad_y += cell_h;
  }

  for (int r = 0; r < WC_ROWS; r++) {
    for (int c = 0; c < WC_COLS; c++) {
      graphics_context_set_text_color(ctx, s_on[r][c] ? s_settings.highlight : s_settings.text);
      GRect cell = GRect(pad_x + c * cell_w, pad_y + r * cell_h + text_dy, cell_w, cell_h);
      graphics_draw_text(ctx, wc_grid[r][c], s_font, cell,
                         GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    }
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
