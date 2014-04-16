// Standard includes
#include "pebble.h"

#define DAY_FRAME       (GRect(0, 2, 144, 50))
#define TIME_FRAME      (GRect(0, 18, 144, 168-6))
#define DATE_FRAME      (GRect(0, 66, 144-4, 168-62))

#define DELTA_FRAME     (GRect(4, 100, 144-8, 30))
#define TEXT_FRAME      (GRect(4, 125, 144-8, 168-125))

#define INVERTER_FRAME  (GRect(0, 90, 144, 168-90))

#define DELTA_T_PKEY 1

#define MAX_RECORDS 30

#define VIBRATE_PKEY 100

typedef struct {
  char name[32];
  time_t  target_time;
} EventRecord;

static EventRecord records[MAX_RECORDS];
static int total_records;
static int current_record;
static int num_records;
static bool vibrate_on_switch;

enum {
  KEY_NAME = 0,
  KEY_TARGET,
  KEY_REQUEST_UPDATE,
  KEY_REQUEST_RESET,
  KEY_LENGTH,
  KEY_VIBRATE
};

// App-specific data
Window *window;
TextLayer *time_layer;
TextLayer *date_layer;
TextLayer *day_layer;
TextLayer *delta_layer;
TextLayer *text_layer;

static InverterLayer *inverter_layer;

GFont font_time;
GFont font_subhead;
GFont font_text;

static void requestUpdate();

static void show_delta() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "%s (%d of %d)", __PRETTY_FUNCTION__, current_record, num_records);

  if (num_records == 0) {
    text_layer_set_text(delta_layer, "No countdowns.");
    text_layer_set_text(text_layer, "Add using Settings");
    return;
  }
  static char delta_text[32];
  /*time_t delta_t = persist_exists(DELTA_T_PKEY) ? persist_read_int(DELTA_T_PKEY) : 0;*/
  time_t delta_t = records[current_record].target_time;
  time_t now = time(NULL);
  localtime(&now);

  time_t elapsedSec;
  if (delta_t >= now) {
    elapsedSec = difftime( delta_t , now );
  }
  else {
    elapsedSec = difftime( now, delta_t );
  }
  struct tm * ptm = gmtime( &elapsedSec );

  snprintf(delta_text, sizeof(delta_text), "%d days",
      ptm->tm_yday);

  text_layer_set_text(delta_layer, delta_text);
  static char until_text[32];
  if (delta_t > now) {
    snprintf(until_text, sizeof(until_text), "Until %s", records[current_record].name);
  }
  else {
    snprintf(until_text, sizeof(until_text), "Since %s", records[current_record].name);
  }

  text_layer_set_text(text_layer, until_text);
}

// Called once per second
static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {

  static char time_text[] = "00:00";
  static char date_text[] = "Xxxxxxxxxxxxxx XX";
  static char day_text[] = "Xxxxxxxxxxx";

  char *time_format;

  if (units_changed & DAY_UNIT) {
    strftime(day_text, sizeof(day_text), "%A", tick_time);
    text_layer_set_text(day_layer, day_text);

    strftime(date_text, sizeof(date_text), "%B %e", tick_time);
    text_layer_set_text(date_layer, date_text);
  }

  if (units_changed & MINUTE_UNIT) {
    if (clock_is_24h_style()) {
      time_format = "%R";
    } else {
      time_format = "%I:%M";
    }

    strftime(time_text, sizeof(time_text), time_format, tick_time);
    /*requestUpdate();*/

    // Kludge to handle lack of non-padded hour format string
    // for twelve hour clock.
    if (!clock_is_24h_style() && (time_text[0] == '0')) {
      memmove(time_text, &time_text[1], sizeof(time_text) - 1);
    }

    text_layer_set_text(time_layer, time_text);

    show_delta();
  }
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "%s", __PRETTY_FUNCTION__);

  Tuple *name_tuple = dict_find(iter, KEY_NAME);
  Tuple *reset_tuple = dict_find(iter, KEY_REQUEST_RESET);
  Tuple *vibrate_tuple = dict_find(iter, KEY_VIBRATE);

  if (reset_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "received RESET REQUEST");
    Tuple *len_tuple = dict_find(iter, KEY_LENGTH);
    current_record = 0;
    num_records = 0;
    total_records = (int)len_tuple->value->int32;
    if (total_records > MAX_RECORDS) {
      total_records = MAX_RECORDS;
    }
  }
  else if (name_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "received Event %s", name_tuple->value->cstring);
    EventRecord record;
    strncpy(record.name, name_tuple->value->cstring, sizeof(record.name));

    /*bail if no time*/
    Tuple *tuple = dict_find(iter, KEY_TARGET);
    if (!(tuple && tuple->type == TUPLE_CSTRING)) return;

    time_t delta_t = atol(tuple->value->cstring);
    record.target_time = delta_t;
    records[num_records] = record;
    num_records++;

    if (num_records == total_records) {
      show_delta();
    }
  }
  else if (vibrate_tuple) {
   vibrate_on_switch = vibrate_tuple->value->uint8;
   APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate_on_switch %d", vibrate_on_switch);
  }
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped  %i", reason);
}

static void out_sent_handler(DictionaryIterator *sent, void *context) {
  /*APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sent!");*/
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send!  %i", reason);
}

static void requestUpdate() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint8(iter, KEY_REQUEST_UPDATE, 1);
  app_message_outbox_send();
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "%s", __PRETTY_FUNCTION__);

  if (vibrate_on_switch) {
    vibes_short_pulse();
  }
  current_record++;
  if (current_record >= num_records) {
    current_record = 0;
  }
  show_delta();
}

static void app_message_init(void) {
  // Register message handlers
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);
  // Init buffers
  app_message_open(512, 64);
}

static void load_settings(void) {
  vibrate_on_switch = persist_exists(VIBRATE_PKEY) && persist_read_bool(VIBRATE_PKEY);
}

// Handle the start-up of the app
static void do_init(void) {
  load_settings();

  // Create our app's base window
  window = window_create();
  window_stack_push(window, true);
  window_set_background_color(window, GColorBlack);
  Layer *window_layer = window_get_root_layer(window);

  font_subhead = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DROID_SANS_18));
  font_time = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DROID_SANS_BOLD_48));
  font_text = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DROID_SANS_BOLD_18));

  day_layer = text_layer_create(DAY_FRAME);
  text_layer_set_text_color(day_layer, GColorWhite);
  text_layer_set_background_color(day_layer, GColorClear);
  text_layer_set_font(day_layer, font_subhead);
  text_layer_set_text_alignment(day_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(day_layer));

  time_layer = text_layer_create(TIME_FRAME);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, font_time);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(time_layer));

  date_layer = text_layer_create(DATE_FRAME);
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_font(date_layer, font_subhead);
  text_layer_set_text_alignment(date_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(date_layer));

  delta_layer = text_layer_create(DELTA_FRAME);
  text_layer_set_text_color(delta_layer, GColorWhite);
  text_layer_set_background_color(delta_layer, GColorClear);
  text_layer_set_font(delta_layer, font_text);
  text_layer_set_text_alignment(delta_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(delta_layer));

  text_layer = text_layer_create(TEXT_FRAME);
  text_layer_set_text_color(text_layer, GColorWhite);
  text_layer_set_background_color(text_layer, GColorClear);
  text_layer_set_font(text_layer, font_subhead);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(text_layer, GTextOverflowModeFill);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  inverter_layer = inverter_layer_create(INVERTER_FRAME);
  layer_add_child(window_layer, inverter_layer_get_layer(inverter_layer));

  app_message_init();
  requestUpdate();

  // Update the screen right away
  time_t now = time(NULL);
  handle_second_tick(localtime(&now), SECOND_UNIT | MINUTE_UNIT | HOUR_UNIT | DAY_UNIT );
  // And then every second
  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);

  accel_tap_service_subscribe(accel_tap_handler);
}

static void do_deinit(void) {
  persist_write_bool(VIBRATE_PKEY, vibrate_on_switch);

  accel_tap_service_unsubscribe();
  tick_timer_service_unsubscribe();
  text_layer_destroy(time_layer);
  text_layer_destroy(delta_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(day_layer);
  text_layer_destroy(text_layer);
  window_destroy(window);

  fonts_unload_custom_font(font_time);
  fonts_unload_custom_font(font_subhead);
  fonts_unload_custom_font(font_text);
}

// The main event/run loop for our app
int main(void) {
  do_init();
  app_event_loop();
  do_deinit();
}
