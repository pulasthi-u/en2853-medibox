#include "Arduino.h"
#include "WiFi.h"
#include "PubSubClient.h"
#include "mqtt.h"

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);

char* mqtt_topic_payloads[NUM_SUBSCRIBE_TOPICS];
bool mqtt_topics_updated[NUM_SUBSCRIBE_TOPICS];

void mqtt_connect() {
// We implement a blocking version and a non-blocking version of the connect function. The blocking version can be used in  
// setup to run once. The non-blocking version can be used in the main loop to check if the client is connected and reconnect if not,
// without causing delays which could affect timing.s

#ifdef MQTT_BLOCKING_CONNECT
  while (!mqtt_client.connected()) {
    Serial.print("Connecting to MQTT...");
#endif
    
    if (mqtt_client.connect(MQTT_CLIENT_NAME)) {
      // Subscribe to each topic after successful connection
      for (int i = 0; i < NUM_SUBSCRIBE_TOPICS; i++) {
        mqtt_client.subscribe(mqtt_subscribe_topics[i]);
      }
    } /* else {
        delay(2000);
    } */

#ifdef MQTT_BLOCKING_CONNECT
  }
#endif
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    // Check which of the subscribed topics the message is from, and update the corresponding buffers
    for (int i = 0; i < NUM_SUBSCRIBE_TOPICS; i++) {
    if (!strcmp(topic, mqtt_subscribe_topics[i])) {
      mqtt_topics_updated[i] = true;
      mqtt_topic_payloads[i] = (char*)malloc(length + 1);
      memcpy(mqtt_topic_payloads[i], payload, length);
      mqtt_topic_payloads[i][length] = '\0';
    }
  }
}