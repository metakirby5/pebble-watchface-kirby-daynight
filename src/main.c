#include <pebble.h>

static const uint32_t KIRBIES[] = {
  RESOURCE_ID_KBY_BACKDROP,
  RESOURCE_ID_KBY_BALL,
  RESOURCE_ID_KBY_BEAM,
  RESOURCE_ID_KBY_CRASH,
  RESOURCE_ID_KBY_CUTTER,
  RESOURCE_ID_KBY_FIRE,
  RESOURCE_ID_KBY_FIREBALL,
  RESOURCE_ID_KBY_FREEZE,
  RESOURCE_ID_KBY_HAMMER,
  RESOURCE_ID_KBY_HIJUMP,
  RESOURCE_ID_KBY_ICE,
  RESOURCE_ID_KBY_LASER,
  RESOURCE_ID_KBY_LIGHT,
  RESOURCE_ID_KBY_MIKE,
  RESOURCE_ID_KBY_NEEDLE,
  RESOURCE_ID_KBY_PARASOL,
  RESOURCE_ID_KBY_SLEEP,
  RESOURCE_ID_KBY_SPARK,
  RESOURCE_ID_KBY_STONE,
  RESOURCE_ID_KBY_SWORD,
  RESOURCE_ID_KBY_THROW,
  RESOURCE_ID_KBY_TORNADO,
  RESOURCE_ID_KBY_UFO,
  RESOURCE_ID_KBY_WHEEL
};

static Window *s_main_window;

static TextLayer *s_time_layer;
static GFont s_time_font;
  
static BitmapLayer *s_kirby_layer;
static GBitmap *s_kirby;

static void update_time() {
  
  // Time
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  static char buffer[] = "00:00";
  strftime(buffer, sizeof("00:00"), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  
  text_layer_set_text(s_time_layer, buffer);
  
  // Kirby - update every 5 secs
  static int cur_tick = 0;
  static int cur_kirby = 0;
  if (cur_tick > 4) {
    cur_tick = 0;
    
    if (cur_kirby > 23)
      cur_kirby = 0;
    
    gbitmap_destroy(s_kirby);
    s_kirby = gbitmap_create_with_resource(KIRBIES[cur_kirby]);
    bitmap_layer_set_bitmap(s_kirby_layer, s_kirby);
    
    cur_kirby++;
  }
  cur_tick++;
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  
  update_time();
}

static void main_window_load(Window *window) {
  
  // Time
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_EMULOGIC_24));
  
  s_time_layer = text_layer_create(GRect(-4, 138, 148, 30));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Kirby
  s_kirby = gbitmap_create_with_resource(RESOURCE_ID_KBY_MIX);
  s_kirby_layer = bitmap_layer_create(GRect(24, 10, 96, 120));
  bitmap_layer_set_bitmap(s_kirby_layer, s_kirby);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_kirby_layer));
}

static void main_window_unload(Window *window) {
  
  text_layer_destroy(s_time_layer);
  fonts_unload_custom_font(s_time_font);
  
  bitmap_layer_destroy(s_kirby_layer);
  gbitmap_destroy(s_kirby);
}

static void init() {
  
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorWhite);
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(s_main_window, true);
  update_time();
}

static void deinit() {
  
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
