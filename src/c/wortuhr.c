#include <pebble.h>

#include "wordclock.h"

static Window *s_window;
static Layer *s_grid_layer;
static GFont s_font;
static int s_font_height;
static bool s_on[WC_ROWS][WC_COLS];

static void grid_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  int cell_w = bounds.size.w / WC_COLS;
  int cell_h = bounds.size.h / WC_ROWS;
  int pad_x = (bounds.size.w - cell_w * WC_COLS) / 2;
  int pad_y = (bounds.size.h - cell_h * WC_ROWS) / 2;

  // graphics_draw_text setzt oberhalb der Glyphe Luft an; die ziehen wir wieder ab.
  int text_dy = (cell_h - s_font_height) / 2 - 3;

  for (int r = 0; r < WC_ROWS; r++) {
    for (int c = 0; c < WC_COLS; c++) {
      graphics_context_set_text_color(ctx,
          s_on[r][c] ? GColorBlack : PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite));
      GRect cell = GRect(pad_x + c * cell_w, pad_y + r * cell_h + text_dy, cell_w, cell_h);
      graphics_draw_text(ctx, wc_grid[r][c], s_font, cell,
                         GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    }
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  wc_compute(tick_time->tm_hour, tick_time->tm_min, s_on);
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
  s_window = window_create();
  window_set_background_color(s_window, GColorWhite);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
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
