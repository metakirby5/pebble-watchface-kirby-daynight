#include <pebble.h>
  
#define NUM_KIRBIES 24

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

static int cur_kirby;

static void update_time() {
  
  // Time
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  static char buffer[] = "00:00";
  strftime(buffer, sizeof("00:00"), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  
  text_layer_set_text(s_time_layer, buffer);
  
  // Set composting mode based on day/night
  static bool day = true;
  
  // Night and it turns to day?
  if (!day && (tick_time->tm_hour >= 6 && tick_time->tm_hour < 18)) {
    bitmap_layer_set_compositing_mode(s_kirby_layer, GCompOpAssign);
    text_layer_set_text_color(s_time_layer, GColorBlack);
    window_set_background_color(s_main_window, GColorWhite);
    day = true;
  }
  // Day and it turns to night?
  else if (day && (tick_time->tm_hour >= 18 || tick_time->tm_hour < 6)) {
    bitmap_layer_set_compositing_mode(s_kirby_layer, GCompOpAssignInverted);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    window_set_background_color(s_main_window, GColorBlack);
    day = false;
  }
  
  // Kirby
  if (s_kirby)
    gbitmap_destroy(s_kirby);
  s_kirby = gbitmap_create_with_resource(KIRBIES[cur_kirby]);
  bitmap_layer_set_bitmap(s_kirby_layer, s_kirby);
  
  cur_kirby++;
  if (cur_kirby > NUM_KIRBIES - 1)
    cur_kirby = 0;
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
  s_kirby_layer = bitmap_layer_create(GRect(24, 10, 96, 120));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_kirby_layer));
  
  srand(time(NULL));
  cur_kirby = rand() % NUM_KIRBIES;
  
  update_time();
  
  // Go back one Kirby so the next update keeps it the same
  cur_kirby = (cur_kirby - 1 + NUM_KIRBIES) % NUM_KIRBIES;
}

static void main_window_unload(Window *window) {
  
  text_layer_destroy(s_time_layer);
  fonts_unload_custom_font(s_time_font);
  
  bitmap_layer_destroy(s_kirby_layer);
  if (s_kirby)
    gbitmap_destroy(s_kirby);
}

static void init() {
  
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorWhite);
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(s_main_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
