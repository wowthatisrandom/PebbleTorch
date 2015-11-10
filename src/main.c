#include <pebble.h>
       
Window *my_window;
bool is_light_on;
bool is_bg_white;
static bool is_battery_showing;
static BitmapLayer *pow_layer;
TextLayer *battery_layer;
GBitmap *pow_white;
GBitmap *pow_black;
 
 
 
static void deinit(void) {
	tick_timer_service_unsubscribe();
	accel_tap_service_unsubscribe();
	battery_state_service_unsubscribe();
	bitmap_layer_destroy(pow_layer);
	text_layer_destroy(battery_layer);
	window_destroy(my_window);
}
 
static void window_load(Window *window){
    window_set_background_color(window, GColorWhite);
    light_enable(true);
    is_light_on = true;
    is_bg_white = true;
}
 
static void window_unload(Window *window){
    light_enable(false);
    is_light_on = false;
}
 
static void handle_battery(BatteryChargeState charge_state) {
	static char battery_text[] = "100%";
	
	if (charge_state.is_charging) {
	  snprintf(battery_text, 9, "charging");
	} else {
	  snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
	}
  	text_layer_set_text(battery_layer, battery_text);
}
 
static void tick_handler(struct tm *t, TimeUnits units_changed){
	light_enable(true);
    handle_battery(battery_state_service_peek());
}
 
void toggle_light() {
	if(is_light_on) {
		light_enable(false);
        tick_timer_service_unsubscribe();
        is_light_on = false;
	} else {
		tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
		is_light_on = true;
    }
}
 
void toggle_bg(Window *window) {
	if(is_bg_white) {
		is_bg_white = false;
        window_set_background_color(window, GColorWhite);
        text_layer_set_text_color(battery_layer, GColorBlack);
        bitmap_layer_set_bitmap(pow_layer, pow_white);
     } else {
        is_bg_white = true;
        window_set_background_color(window, GColorBlack);
        text_layer_set_text_color(battery_layer, GColorWhite);
        bitmap_layer_set_bitmap(pow_layer, pow_black);
     }
}
 
void show_batt(){
	if(is_battery_showing) {
		layer_set_hidden(text_layer_get_layer(battery_layer), true);
		is_battery_showing = false;
    } else {
        layer_set_hidden(text_layer_get_layer(battery_layer), false);
        is_battery_showing = true;
    }
}
 
static void handle_accel(AccelAxisType axis, int32_t direction) {
    toggle_light();
}
 
void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	toggle_light();
}
 
void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	Window* window = (Window*)context;
    toggle_bg(window);
}
 
void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	show_batt();
}
 
void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	layer_set_hidden(bitmap_layer_get_layer(pow_layer), false);
}
 
void down_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {
	layer_set_hidden(bitmap_layer_get_layer(pow_layer), true);
}
 
void config_provider(Window *window) {
	window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
	window_long_click_subscribe(BUTTON_ID_DOWN, 500, down_long_click_handler, down_long_click_release_handler);
}
 
static void init(void) {
	my_window = window_create();
    WindowHandlers handlers = {
		.load = window_load,
        .unload = window_unload
    };
	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
	accel_tap_service_subscribe(handle_accel);
	battery_state_service_subscribe(handle_battery);
	window_set_click_config_provider(my_window,(ClickConfigProvider) config_provider);
    window_set_fullscreen(my_window, true);
    window_stack_push(my_window, true);  
    toggle_light();
       
	//create easter egg
	pow_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_POW);
	pow_black = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_POW_BLACK);
	pow_layer = bitmap_layer_create(GRect(0,0,144,168));
	bitmap_layer_set_bitmap(pow_layer, pow_white);
	layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(pow_layer));
	layer_set_hidden(bitmap_layer_get_layer(pow_layer), true);
       
        //create battery meter
    battery_layer = text_layer_create(GRect(0, 0, 140, 34));
    text_layer_set_text_color(battery_layer, GColorBlack);
    text_layer_set_background_color(battery_layer, GColorClear);
    text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text_alignment(battery_layer, GTextAlignmentRight);
    text_layer_set_text(battery_layer, "100%");
    layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(battery_layer));
    layer_set_hidden(text_layer_get_layer(battery_layer), true);
    is_battery_showing = false;
}
 
int main(void) {
    init();
    app_event_loop();
    deinit();
}