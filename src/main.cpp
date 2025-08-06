#include <Arduino.h>

#include <math.h>
#include <cstring>

#include <ESP32Servo.h>
#include <ldr/ldr.h>
#include <lcd/lcd.h>
#include <mqtt/mqtt.h>
#include <time_alarm/time_alarm.h>
#include <navigation/navigation.h>
#include <temp_humidity/temp_humidity.h>

#define BTN_UP                            16
#define BTN_OK                            4
#define BTN_DOWN                          2

#define SERVO_PIN                         23

#define DEF_LDR_SEND_INTERVAL             2 * 60 * 1000 // 2 minutes
#define DEF_TEMP_HUMIDITY_SEND_INTERVAL   2 * 60 * 1000 // 2 minutes

#define DEF_THETA_OFFSET                  30
#define DEF_CONTROLLING_FACTOR            0.75
#define DEF_IDEAL_TEMPERATURE             30

#define THETA_OFFSET_TOPIC                0
#define CONTROLLING_FACTOR_TOPIC          1
#define IDEAL_TEMPERATURE_TOPIC           2
#define LDR_SAMPLE_INTERVAL_TOPIC         3
#define LDR_SEND_INTERVAL_TOPIC           4
#define TEMP_HUMIDITY_SEND_INTERVAL_TOPIC 5

Button ok_btn = {BTN_OK, 0};
Button up_btn = {BTN_UP, 0};
Button down_btn = {BTN_DOWN, 0};
Button *buttons[] = {&ok_btn, &up_btn, &down_btn};

Alarm alarms[NUM_ALARMS];

long ldr_send_interval = DEF_LDR_SEND_INTERVAL;
long last_ldr_send_time = 0;

int max_ldr_samples = ldr_send_interval / ldr_sample_interval;

long temp_humidity_send_interval = DEF_TEMP_HUMIDITY_SEND_INTERVAL;
long last_temp_humidity_send_time = 0;

float theta_offset = DEF_THETA_OFFSET;
float controlling_factor = DEF_CONTROLLING_FACTOR;
float T_med = DEF_IDEAL_TEMPERATURE;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHTesp dht;
Servo shade_control;

char* mqtt_subscribe_topics[NUM_SUBSCRIBE_TOPICS] = {
    (char*)"settings/thta_offst",
    (char*)"settings/ctrl_fac",
    (char*)"settings/ideal_temp",
    (char*)"settings/ldr_sample_intl",
    (char*)"settings/ldr_send_intl",
    (char*)"settings/temp_humidity_send_intl"
};


void setup() {
  // Initialize the display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Wait for a while for Adafruit's welcome message to clear
  delay(5000);
  display.clearDisplay();
  
  // Initialize the buttons
  for (int i = 0; i < 3; i++) {
    pinMode(buttons[i]->pin, INPUT);
  }

  // Initialize buzzer, LDR and servo
  pinMode(BUZZER, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  shade_control.attach(SERVO_PIN);
  
  // Initialize the DHT sensor
  dht.setup(DHT_PIN, DHTesp::DHT22);

  // Connect to WiFi
  WiFi.begin("Wokwi-GUEST", "", 6);

  write_to_display("Connecting to WiFi...", 1, (SCREEN_HEIGHT / 2) - 4, 1);
  display.display();
  
  // Wait for successful connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }

  // Once connected, update time from the NTP server
  configTime(utc_offset, UTC_OFFSET_DST, NTP_SERVER);

  write_to_display("Done!", 1, (SCREEN_HEIGHT / 2) - 4 + 10, 1);
  display.display();
  
  delay(500);
  display.clearDisplay();

  // Initialize the MQTT client
  mqtt_client.setServer(MQTT_SERVER, 1883);
  mqtt_client.setCallback(mqtt_callback);

  // Attempt a blocking connection to the MQTT broker here, to avoid delays in the main loop
  #define MQTT_BLOCKING_CONNECT
  mqtt_connect();
  #undef MQTT_BLOCKING_CONNECT
}

void main_update() {
  // Update time
  if (!getLocalTime(&time_info)) {
    Serial.println("Failed to obtain time");
    return;
  }

  // Check if an alarm is due
  bool alarm_ringing = false;

  for (int alarm_index = 0; alarm_index < NUM_ALARMS; alarm_index++) {
    if (is_alarm_due(alarm_index)) {
      set_alarm_state(alarm_index, RINGING);
      alarm_ringing = true;

      next_state = next_state * 100 + current_mode * 10 + alarm_index;
      current_mode = ALARM_RING;
    } else if (is_alarm_ringing(alarm_index)) {
      alarm_ringing = true;
    }
  }

  // Update and check if temperature and humidity are in limits
  update_temp_humidity();

  if (!alarm_ringing) { // Do not activate/deactivate the buzzer for temperature/humidity if an alarm is already ringing
    if (temp_humidity_in_limits()) {
      disable_buzzer();
    } else {
      if (!buzzer_enabled) {
        enable_buzzer(500);
      }
    }
  }

  // MQTT Loop
  mqtt_client.loop();

  // Check for incoming MQTT messages, and update corresponding variables
  for (int i = 0; i < NUM_SUBSCRIBE_TOPICS; i++) {
    // We check if there has been an update in any topic
    if (mqtt_topics_updated[i]) {      
      mqtt_topics_updated[i] = false; // Reset the update status for the next update will trigger it

      if (i == THETA_OFFSET_TOPIC) {
        theta_offset = atof(mqtt_topic_payloads[i]);
      } else if (i == CONTROLLING_FACTOR_TOPIC) {
        controlling_factor = atof(mqtt_topic_payloads[i]);
      } else if (i == IDEAL_TEMPERATURE_TOPIC) {
        T_med = atof(mqtt_topic_payloads[i]);
      } else if ((i == LDR_SAMPLE_INTERVAL_TOPIC) || (i == LDR_SEND_INTERVAL_TOPIC)) {
        if (i == LDR_SAMPLE_INTERVAL_TOPIC) {
          ldr_sample_interval = atoi(mqtt_topic_payloads[i]);
        } else if (i == LDR_SEND_INTERVAL_TOPIC) {
          ldr_send_interval = atoi(mqtt_topic_payloads[i]);
        }

        // Calculate the new number of samples in a send-interval
        int new_max_ldr_samples = ldr_send_interval / ldr_sample_interval;

        if (new_max_ldr_samples > max_ldr_samples) {
          // If more samples are to be taken, simply expand the array, copying the existing values to the front
          ldr_samples = (float*)realloc(ldr_samples, new_max_ldr_samples * sizeof(float));

          ldr_sample_index = max_ldr_samples; // So that we start filling the new array from the new empty position now introduced
        } else if (new_max_ldr_samples < max_ldr_samples) {
          // If fewer samples are to be taken, form a new array with the as many of the most recent readings as possible
          float* new_ldr_samples = (float*)malloc(new_max_ldr_samples * sizeof(float));

          // We must recalculate the average using only the values that are going to remain in the new array
          ldr_average = 0;
          
          // Because maybe, originally, the array was supposed to be big, but still does not have as many entries as the new smaller
          // array is required to have; in that case ldr_sample_index is the number of samples still in the array, so use that.
          int num_valid_samples = min(ldr_sample_index, new_max_ldr_samples);

          // Copy the most recent samples to the new array, so that the older entries appear in front
          for (int i = 0; i < num_valid_samples; i++) {
            int next_oldest_sample_index = (ldr_sample_index - num_valid_samples + i) % max_ldr_samples;
            new_ldr_samples[i] = ldr_samples[next_oldest_sample_index];
            ldr_average += new_ldr_samples[i];
          }

          ldr_average /= new_max_ldr_samples;

          free(ldr_samples);
          ldr_samples = new_ldr_samples;
          
          ldr_sample_index = new_max_ldr_samples; // So that we start filling the new array from the beginning
        }

        max_ldr_samples = new_max_ldr_samples;
      } else if (i == TEMP_HUMIDITY_SEND_INTERVAL_TOPIC) {
        temp_humidity_send_interval = atoi(mqtt_topic_payloads[i]);
      }
    }
  }

  // Sample and send LDR data
  sample_ldr();

  if (millis() - last_ldr_send_time > ldr_send_interval) {
    mqtt_client.publish("sensor/ldr", String(ldr_average).c_str());
    last_ldr_send_time = millis();
  }

  // Send temperature and humidity data
  if (millis() - last_temp_humidity_send_time > temp_humidity_send_interval) {
    mqtt_client.publish("sensor/temp", String(current_temperature).c_str());
    mqtt_client.publish("sensor/humidity", String(current_humidity).c_str());
    
    last_temp_humidity_send_time = millis();
  }

  // Rotate the servo
  int theta = theta_offset
                + (180 - theta_offset) * light_intensity * controlling_factor
                * log((float)ldr_sample_interval / (float)ldr_send_interval)
                * (current_temperature / T_med);
  
  shade_control.write((int)theta);
}

void loop() {
  // Clear the display
  display.clearDisplay();

  // Most core functionality is handled here
  main_update();

  // Check if the MQTT client is connected, and reconnect if not
  if (!mqtt_client.connected()) {
    mqtt_connect();
  }
  
  // Read button states
  for (int i = 0; i < 3; i++) {
    buttons[i]->state = digitalRead(buttons[i]->pin);
  }
  delay(100); // For debounce

  // Write the right content on screen based on the current mode
  if (current_mode == HOME) {
    home_screen();
  } else if (current_mode == MENU) {
    menu();
  } else if (current_mode == TIME_SET) {
    next_state = std::find(timezones_numerical, timezones_numerical + NUM_TIMEZONES, utc_offset) - timezones_numerical;
    time_set();
  } else if (current_mode == ALARM_LIST) {
    alarm_list();
  } else if (current_mode == ALARM_SET) {
    current_mode = ALARM_LIST;
    next_state = ALARM_SET;
  } else if (current_mode == ALARM_VIEW_DELETE) {
    current_mode = ALARM_LIST;
    next_state = ALARM_VIEW_DELETE;
  } else if (current_mode == ALARM_EDIT) {
    alarm_set();
  } else if (current_mode == ALARM_DATA) {
    alarm_view();
  }else if (current_mode == ALARM_RING) {
    alarm_ring();
  }

  // Display the written content
  display.display();

  // Activate the buzzer if needed
  buzzer();
}