#ifndef _MQTT_H_
#define _MQTT_H_

#include "Arduino.h"
#include "WiFi.h"
#include "PubSubClient.h"

#define MQTT_CLIENT_NAME        "MEDIBOX_220658U"
#define MQTT_SERVER             "test.mosquitto.org"
#define MQTT_PORT               1883

#define NUM_SUBSCRIBE_TOPICS    6

extern WiFiClient wifi_client;
extern PubSubClient mqtt_client;

extern char* mqtt_subscribe_topics[NUM_SUBSCRIBE_TOPICS]; // cpp type string?
extern char* mqtt_topic_payloads[NUM_SUBSCRIBE_TOPICS];
extern bool mqtt_topics_updated[NUM_SUBSCRIBE_TOPICS];

void mqtt_connect();
void mqtt_callback(char* topic, byte* payload, unsigned int length);

#endif