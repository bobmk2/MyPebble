#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

static Window *s_main_window;
static TextLayer *s_weather_layer;
static TextLayer *s_time_layer;
static GFont s_time_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static GFont s_weather_font;

// 矩形とか書くようのレイヤー
static Layer *s_over_layer;

static TextLayer *s_home_chance_layer; // 自宅の降水確率
static TextLayer *s_work_chance_layer; // 職場の降水確率

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 2h hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    //Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);

	// 30分毎に更新
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

// 降水確率の文字列初期化
static void init_rain_chance_texts(Window *window){
	s_home_chance_layer = text_layer_create(GRect(9, 17, 47, 28));
	text_layer_set_background_color(s_home_chance_layer, GColorClear);
  text_layer_set_text_color(s_home_chance_layer, GColorBlack);
	text_layer_set_text_alignment(s_home_chance_layer, GTextAlignmentCenter);
  text_layer_set_text(s_home_chance_layer, "99%");
//	text_layer_set_font(s_home_chance_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PIXEL_MPLUS10_REGULAR_46)));
	text_layer_set_font(s_home_chance_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_26)));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_home_chance_layer));
	

}


// エリアの内側を埋める
static void draw_fill_areas( GContext* ctx, int x, int y, int value ) {
		for(int i = 0; i < 9; i ++){
			int t = 1 << i;
			
			int a = (i % 3) * 6;
			int b = (i / 3) * 6;
			
			if ( value & t ) {
				graphics_fill_rect(ctx, GRect( x + a,y + b,3,3), 0, GCornerNone);
			}
		}
}

// 降水量を埋める
static void draw_fill_amounts( GContext* ctx, int x, int y, int amount) {
	for( int i = 0; i < amount; i++ ){
		graphics_fill_rect(ctx, GRect( x + (i*5), y, 4, 6), 0, GCornerNone);
	}
}

static void draw_test(Layer *layer, GContext* ctx) {
	
	time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
	
	
	// 降水確率は既に記述済みとする
	draw_fill_areas(ctx, 64,35, 341);
	
	draw_fill_amounts( ctx, 112, 31, 1 );
	draw_fill_amounts( ctx, 112, 40, 3 );
	draw_fill_amounts( ctx, 112, 49, 5 );
	
}


static void main_window_load(Window *window) {	
	// Create GBitmap, then set to created BitmapLayer
	s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RAIN_SIGHTS_BACKGROUND);
	s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));

	s_over_layer = layer_create(GRect(0,0,144,168));
	layer_set_update_proc( s_over_layer, &draw_test);
	layer_add_child(window_get_root_layer(window), s_over_layer);
	
	// Create time TextLayer
  s_time_layer = text_layer_create(GRect(5, 52, 139, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
	
	// 降水確率の文字列初期化
	init_rain_chance_texts(window);

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Make sure the time is displayed from the start
  update_time();
	
	// Create GFont
	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));

	// Apply to TextLayer
	text_layer_set_font(s_time_layer, s_time_font);

	
		// Create temperature Layer
	s_weather_layer = text_layer_create(GRect(0, 130, 144, 25));
	text_layer_set_background_color(s_weather_layer, GColorClear);
	text_layer_set_text_color(s_weather_layer, GColorBlack);
	text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
	text_layer_set_text(s_weather_layer, "Loading...");
	
	
	// Create second custom font, apply it and add to Window
	s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_20));
	text_layer_set_font(s_weather_layer, s_weather_font);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
	
	// Unload GFont
fonts_unload_custom_font(s_time_font);
	
	// Destroy GBitmap
gbitmap_destroy(s_background_bitmap);

// Destroy BitmapLayer
bitmap_layer_destroy(s_background_layer);
	
	// Destroy weather elements
text_layer_destroy(s_weather_layer);
fonts_unload_custom_font(s_weather_font);
	
	text_layer_destroy(s_home_chance_layer);
	layer_destroy(s_over_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

// ==============================================================
// コールバック
// ==============================================================
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	// js側からの情報を保持
	static char temperature_buffer[8];		// 温度
	static char conditions_buffer[32];		// 天気
	static char weather_layer_buffer[32];
	
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    switch(t->key) {
			case KEY_TEMPERATURE:
				snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)t->value->int32);
				break;
			case KEY_CONDITIONS:
				snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
				break;
			default:
				APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
				break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
	
	// 自宅情報の記述
	text_layer_set_text(s_home_chance_layer, "50%");

	// Assemble full string and display
	snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
	text_layer_set_text(s_weather_layer, weather_layer_buffer);
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


// ==============================================================
// 初期化
// ==============================================================
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	
	// コールバックの設定
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);

	
	// Open AppMessage
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}



// ==============================================================
// メイン
// ==============================================================
int main(void) {
  init();
  app_event_loop();
  deinit();
}
