#include "pebble.h"
#include "main.h" // Used for drawing hands
#include "gcolor_definitions.h" // Allows the use of colors such as "GColorMidnightGreen"

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

static Window *s_main_window; 
static Layer *s_solid_bg_layer, *s_hands_layer;
static BitmapLayer *s_background_layer; 
static GBitmap *s_background_bitmap; 
static GPath *s_minute_arrow, *s_hour_arrow, *s_second_arrow; 
static TextLayer *s_date_layer, *s_month_layer,*s_weather_layer, *s_temperature_layer, *s_step_layer;
int cx, cy, xDate, yDate, xStep, yStep, xMonth, yMonth, xWeather, yWeather;
GPoint center;

static char s_current_steps_buffer[16];
static int s_step_count = 0, s_step_goal = 0, s_step_average = 0;


static void bg_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone); 
  
  //Get Center
  GRect bounds = layer_get_bounds(layer); 
  cx = bounds.size.w / 2;
  cy = bounds.size.h / 2;
  center = GPoint(cx, cy);
      
  //Month Background
  xMonth = PBL_IF_ROUND_ELSE(cx - 47, cx - 47);
  yMonth = PBL_IF_ROUND_ELSE(45, 32);
  graphics_context_set_fill_color(ctx, GColorRed); 
  graphics_fill_circle(ctx, GPoint(xMonth, yMonth), 20);
  
  //Day Background
  xDate = PBL_IF_ROUND_ELSE(cx + 47, cx + 47);
  yDate = PBL_IF_ROUND_ELSE(45, 32);
  graphics_context_set_fill_color(ctx, GColorIslamicGreen); 
  graphics_fill_circle(ctx, GPoint(xDate, yDate), 20);
  
  //Step Background
  xStep = PBL_IF_ROUND_ELSE(cx + 47, cx + 47);
  yStep = PBL_IF_ROUND_ELSE(135, 140);
  graphics_context_set_fill_color(ctx, GColorFashionMagenta); 
  graphics_fill_circle(ctx, GPoint(xStep, yStep), 20);
  
  //Weather Background
  xWeather = PBL_IF_ROUND_ELSE(cx - 47, cx - 47);
  yWeather = PBL_IF_ROUND_ELSE(135, 140);
  graphics_context_set_fill_color(ctx, GColorBlueMoon); 
  graphics_fill_circle(ctx, GPoint(xWeather, yWeather), 20);
  
  //dot top center
  graphics_context_set_fill_color(ctx, GColorOrange); 
  graphics_fill_circle(ctx, GPoint(cx, PBL_IF_ROUND_ELSE(25,23)), 14);
  
  //dot bottom center
  graphics_context_set_fill_color(ctx, GColorVividViolet); 
  graphics_fill_circle(ctx, GPoint(cx, PBL_IF_ROUND_ELSE(155, 145)), 14);
   
  //dot left
  graphics_context_set_fill_color(ctx, GColorCyan); 
  graphics_fill_circle(ctx, GPoint(PBL_IF_ROUND_ELSE(25,15), cy), 14);
  
  //dot right
  graphics_context_set_fill_color(ctx, GColorYellow); 
  graphics_fill_circle(ctx, GPoint(PBL_IF_ROUND_ELSE(155, 129), cy), 14);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char temperature_layer_buffer[32];

  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);

  // If all data is available, use it
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°", (int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);

    //Temperature Layer
    snprintf(temperature_layer_buffer, sizeof(temperature_buffer), "%s", temperature_buffer);
    text_layer_set_text(s_temperature_layer, temperature_layer_buffer);
    // Assemble full string and display
    //snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    //text_layer_set_text(s_weather_layer, weather_layer_buffer);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


// Health Data Start
bool step_data_is_available() {
  return HealthServiceAccessibilityMaskAvailable &
    health_service_metric_accessible(HealthMetricStepCount,
      time_start_of_today(), time(NULL));
}

// Daily step goal
static void get_step_goal() {
  const time_t start = time_start_of_today();
  const time_t end = start + SECONDS_PER_DAY;
  s_step_goal = (int)health_service_sum_averaged(HealthMetricStepCount,
    start, end, HealthServiceTimeScopeDaily);
}

// Todays current step count
static void get_step_count() {
  s_step_count = (int)health_service_sum_today(HealthMetricStepCount);
}

// Average daily step count for this time of day
static void get_step_average() {
  const time_t start = time_start_of_today();
  const time_t end = time(NULL);
  s_step_average = (int)health_service_sum_averaged(HealthMetricStepCount,
    start, end, HealthServiceTimeScopeDaily);
}

static void display_step_count() {
  int thousands = s_step_count / 1000;
  int hundreds = s_step_count % 1000;
 
  text_layer_set_text_color(s_step_layer, GColorWhite);
  
    if(thousands > 0) {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%dk", thousands);
  } else {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%d", hundreds);   
  }
  
  text_layer_set_text(s_step_layer, s_current_steps_buffer);
}

static void health_handler(HealthEventType event, void *context) {
  if(event == HealthEventSignificantUpdate) {
    get_step_goal();
  }
  if(event != HealthEventSleepUpdate) {
    get_step_count();
    get_step_average();
    display_step_count();
  }
}
// Health Data End

static void hands_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL); 
  struct tm *t = localtime(&now); 
  
  static char s_buffer_date[8]; 
  strftime(s_buffer_date, sizeof("dd/MMM/yy"), "%e", t);
  text_layer_set_text(s_date_layer, s_buffer_date);
  
  static char s_buffer_month[3];
  strftime(s_buffer_month, sizeof("dd/MMM/yy"), "%a", t); 
  text_layer_set_text(s_month_layer, s_buffer_month);
    
  // Clock Background Circle
  //graphics_context_set_fill_color(ctx, GColorWhite);  
  //graphics_fill_circle(ctx, center, 40);
  graphics_context_set_stroke_width(ctx, 6); 
  graphics_context_set_stroke_color(ctx, GColorDukeBlue); 
  graphics_draw_circle(ctx, GPoint(cx, cy), 13); 
  graphics_context_set_stroke_color(ctx, GColorBlueMoon);
  graphics_draw_circle(ctx, GPoint(cx, cy), 25);  
  graphics_context_set_stroke_color(ctx, GColorPictonBlue);
  graphics_draw_circle(ctx, GPoint(cx, cy), 38);
  
  // Set stroke width/color for hands
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 1);
  
  // Minute hand
  graphics_context_set_fill_color(ctx, GColorWhite); 
  gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
  gpath_draw_filled(ctx, s_minute_arrow); 
  gpath_draw_outline(ctx, s_minute_arrow);
  
  // Hour hand
  graphics_context_set_fill_color(ctx, GColorWhite); 
  gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6)); 
  gpath_draw_filled(ctx, s_hour_arrow);
  gpath_draw_outline(ctx, s_hour_arrow);  
  
  //Second hand
  //graphics_context_set_fill_color(ctx, GColorRed); 
  //gpath_rotate_to(s_second_arrow, (TRIG_MAX_ANGLE * t->tm_sec / 60)); 
  //gpath_draw_filled(ctx, s_second_arrow);  
  
  // Center dot  
  graphics_context_set_fill_color(ctx, GColorRed); 
  graphics_fill_circle(ctx, center, 5);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, GPoint(cx, cy), 5);
  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(s_main_window));
  
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void window_load(Window *s_main_window) {
  Layer *window_layer = window_get_root_layer(s_main_window); 
  GRect bounds = layer_get_bounds(window_layer); 

  s_solid_bg_layer = layer_create(bounds); 
  s_background_layer = bitmap_layer_create(bounds); 
  s_hands_layer = layer_create(bounds); 
  
  layer_set_update_proc(s_solid_bg_layer, bg_update_proc); 
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap); 
  layer_set_update_proc(s_hands_layer, hands_update_proc); 
  
  #if PBL_PLATFORM_BASALT // Only set this for SDK 3.0 +
    bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet); // Set background layer to be transparent
  #endif
  
  layer_add_child(window_layer, s_solid_bg_layer); 
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer)); 
  layer_add_child(window_layer, s_hands_layer); 
  
   // Create MonthLayer
  s_month_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(-47, -47), PBL_IF_ROUND_ELSE(30, 17), bounds.size.w, 50));
  text_layer_set_background_color(s_month_layer, GColorClear);
  text_layer_set_text_color(s_month_layer, GColorWhite);  
  text_layer_set_font(s_month_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_month_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_month_layer));
  
  
  // Create DateLayer
  s_date_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(45, 46), PBL_IF_ROUND_ELSE(30, 17), bounds.size.w, 50));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);  
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  
   // Create a layer to hold the current step count
  s_step_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(47,47), PBL_IF_ROUND_ELSE(120, 125), bounds.size.w, 50));
  text_layer_set_background_color(s_step_layer, GColorClear);
  text_layer_set_text_color(s_step_layer, GColorWhite);
  text_layer_set_font(s_step_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_step_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_step_layer));
  
  // Subscribe to health events if we can
  if(step_data_is_available()) {
    health_service_events_subscribe(health_handler, NULL);
  }
  
  // Create temperature Layer
  s_temperature_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(-44, -44), PBL_IF_ROUND_ELSE(120, 125), bounds.size.w, 25));
  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_text_color(s_temperature_layer, GColorWhite);
  text_layer_set_font(s_temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentCenter);
  text_layer_set_text(s_temperature_layer, "...");
  layer_add_child(window_layer, text_layer_get_layer(s_temperature_layer));
}

static void window_unload(Window *s_main_window) {
  layer_destroy(s_solid_bg_layer); 
  gbitmap_destroy(s_background_bitmap); 
  bitmap_layer_destroy(s_background_layer);
  layer_destroy(s_hands_layer); 
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_month_layer);
  text_layer_destroy(s_step_layer);
  text_layer_destroy(s_weather_layer);
}

static void init() {
  s_main_window = window_create(); 
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load, 
    .unload = window_unload, 
  });
  window_stack_push(s_main_window, true); 

  s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS); 
  s_hour_arrow = gpath_create(&HOUR_HAND_POINTS); 
  s_second_arrow = gpath_create(&SEC_POINTS); 

  Layer *window_layer = window_get_root_layer(s_main_window); 
  GRect bounds = layer_get_bounds(window_layer); 
  GPoint center = grect_center_point(&bounds); 
  gpath_move_to(s_minute_arrow, center); 
  gpath_move_to(s_hour_arrow, center); 
  gpath_move_to(s_second_arrow, center); 

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler); 
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  
  gpath_destroy(s_minute_arrow);
  gpath_destroy(s_hour_arrow); 
  gpath_destroy(s_second_arrow); 
  tick_timer_service_unsubscribe(); 
  window_destroy(s_main_window); 
}

int main() {
  init();
  app_event_loop();
  deinit();
}
