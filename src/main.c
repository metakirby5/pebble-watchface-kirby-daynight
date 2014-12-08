#include <pebble.h>
#include <ctype.h>
  
#define NUM_KIRBIES     24
 
#define INTERVAL_UPDATE MINUTE_UNIT
#define HR_DAY          6
#define COLOR_FG_DAY    GColorBlack
#define COLOR_BG_DAY    GColorWhite
#define COMP_OP_DAY     GCompOpAssign
#define HR_NIGHT        18
#define COLOR_FG_NIGHT  GColorWhite
#define COLOR_BG_NIGHT  GColorBlack
#define COMP_OP_NIGHT   GCompOpAssignInverted
#define FMT_24H         "%H:%M"
#define FMT_12H         "%I:%M"
#define FMT_TIME_LEN    sizeof("00:00")
#define FMT_DATE        "%a %m %d"
#define FMT_DATE_LEN    sizeof("XXX 00 00")

#define PWR_HI          70
#define PWR_MED         40
#define PWR_LO          10

#define POS_TIME        GRect(-4, 140, 148, 30)
#define POS_DATE        GRect(15, 127, 114, 15)
#define POS_BT          GRect(8, 5, 9, 15)
#define POS_BATT        GRect(126, 5, 13, 10)
#define POS_KBY         GRect(24, 5, 96, 120)

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
static Layer *s_root_layer;

static TextLayer *s_time_layer;
static GFont s_time_font;

static TextLayer *s_date_layer;
static GFont s_date_font;
 
static BitmapLayer *s_bluetooth_layer;
static GBitmap *s_bluetooth;

static BitmapLayer *s_battery_layer;
static GBitmap *s_battery;

static BitmapLayer *s_kirby_layer;
static GBitmap *s_kirby;

static int cur_kirby;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  
  // Time
  static char time_buf[FMT_TIME_LEN];
  strftime(time_buf, FMT_TIME_LEN, clock_is_24h_style() ? FMT_24H : FMT_12H, tick_time);
  
  text_layer_set_text(s_time_layer, time_buf);
  layer_mark_dirty(text_layer_get_layer(s_time_layer));
  
  // Date
  static char date_buf[FMT_DATE_LEN];
  strftime(date_buf, FMT_DATE_LEN, FMT_DATE, tick_time);
  char * cptr = date_buf;
  do {
    *cptr = tolower((unsigned char) *cptr);
  } while (*++cptr);
  
  text_layer_set_text(s_date_layer, date_buf);
  layer_mark_dirty(text_layer_get_layer(s_date_layer));
  
  // Set composting mode based on day/night
  static bool day = true;
  
  // Night and it turns to day?
  if (!day && (tick_time->tm_hour >= HR_DAY && tick_time->tm_hour < HR_NIGHT)) {
    bitmap_layer_set_compositing_mode(s_kirby_layer, COMP_OP_DAY);
    bitmap_layer_set_compositing_mode(s_bluetooth_layer, COMP_OP_DAY);
    bitmap_layer_set_compositing_mode(s_battery_layer, COMP_OP_DAY);
    text_layer_set_text_color(s_time_layer, COLOR_FG_DAY);
    text_layer_set_text_color(s_date_layer, COLOR_FG_DAY);
    window_set_background_color(s_main_window, COLOR_BG_DAY);
    layer_mark_dirty(s_root_layer);
    day = true;
  }
  // Day and it turns to night?
  else if (day && (tick_time->tm_hour >= HR_NIGHT || tick_time->tm_hour < HR_DAY)) {
    bitmap_layer_set_compositing_mode(s_kirby_layer, COMP_OP_NIGHT);
    bitmap_layer_set_compositing_mode(s_bluetooth_layer, COMP_OP_NIGHT);
    bitmap_layer_set_compositing_mode(s_battery_layer, COMP_OP_NIGHT);
    text_layer_set_text_color(s_time_layer, COLOR_FG_NIGHT);
    text_layer_set_text_color(s_date_layer, COLOR_FG_NIGHT);
    window_set_background_color(s_main_window, COLOR_BG_NIGHT);
    layer_mark_dirty(s_root_layer);
    day = false;
  }
  
  // Kirby
  if (s_kirby)
    gbitmap_destroy(s_kirby);
  s_kirby = gbitmap_create_with_resource(KIRBIES[cur_kirby]);
  bitmap_layer_set_bitmap(s_kirby_layer, s_kirby);
  layer_mark_dirty(bitmap_layer_get_layer(s_kirby_layer));
  
  cur_kirby++;
  if (cur_kirby > NUM_KIRBIES - 1)
    cur_kirby = 0;
}

static void bt_handler(bool connected) {
  
  if (s_bluetooth)
    gbitmap_destroy(s_bluetooth);
  s_bluetooth = gbitmap_create_with_resource(connected ? RESOURCE_ID_PHONE_BLANK : RESOURCE_ID_PHONE_X);
  bitmap_layer_set_bitmap(s_bluetooth_layer, s_bluetooth);
  layer_mark_dirty(bitmap_layer_get_layer(s_bluetooth_layer));
}

static void batt_handler(BatteryChargeState charge) {
  
  if (s_battery)
    gbitmap_destroy(s_battery);
  s_battery = gbitmap_create_with_resource(
      charge.is_plugged
    ?   !charge.is_charging
    ?      RESOURCE_ID_BATT_OK
    :      RESOURCE_ID_BATT_CHG
    : charge.charge_percent > PWR_HI
    ?   RESOURCE_ID_BATT_3
    : charge.charge_percent > PWR_MED
    ?   RESOURCE_ID_BATT_2
    : charge.charge_percent > PWR_LO
    ?   RESOURCE_ID_BATT_1
    : RESOURCE_ID_BATT_X);
  bitmap_layer_set_bitmap(s_battery_layer, s_battery);
  layer_mark_dirty(bitmap_layer_get_layer(s_battery_layer));
}

static void main_window_load(Window *window) {
  
  s_root_layer = window_get_root_layer(window);
  
  // Time
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_EMULOGIC_24));
  s_time_layer = text_layer_create(POS_TIME);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Date
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_EMULOGIC_12));
  s_date_layer = text_layer_create(POS_DATE);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  // Bluetooth
  s_bluetooth_layer = bitmap_layer_create(POS_BT);
  layer_add_child(s_root_layer, bitmap_layer_get_layer(s_bluetooth_layer));
  
  // Battery
  s_battery_layer = bitmap_layer_create(POS_BATT);
  layer_add_child(s_root_layer, bitmap_layer_get_layer(s_battery_layer));
  
  // Kirby
  s_kirby_layer = bitmap_layer_create(POS_KBY);
  layer_add_child(s_root_layer, bitmap_layer_get_layer(s_kirby_layer));
  
  // === First update ===
  
  // tick_handler
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  srand(temp);
  cur_kirby = rand() % NUM_KIRBIES;
  
  tick_handler(tick_time, INTERVAL_UPDATE);
  
  // Go back one Kirby so the next update keeps it the same
  cur_kirby = (cur_kirby - 1 + NUM_KIRBIES) % NUM_KIRBIES;
  
  // bt_handler
  bt_handler(bluetooth_connection_service_peek());
  
  // batt_handler
  batt_handler(battery_state_service_peek());
}

static void main_window_unload(Window *window) {
  
  text_layer_destroy(s_time_layer);
  fonts_unload_custom_font(s_time_font);
  
  text_layer_destroy(s_date_layer);
  fonts_unload_custom_font(s_date_font);
  
  bitmap_layer_destroy(s_bluetooth_layer);
  gbitmap_destroy(s_bluetooth);
  
  bitmap_layer_destroy(s_battery_layer);
  gbitmap_destroy(s_battery);
  
  bitmap_layer_destroy(s_kirby_layer);
  gbitmap_destroy(s_kirby);
}

static void init() {
  
  tick_timer_service_subscribe(INTERVAL_UPDATE, tick_handler);
  bluetooth_connection_service_subscribe(bt_handler);
  battery_state_service_subscribe(batt_handler);
  
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(s_main_window, true);
}

static void deinit() {
  
  window_destroy(s_main_window);
  
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
}

int main() {
  init();
  app_event_loop();
  deinit();
}
