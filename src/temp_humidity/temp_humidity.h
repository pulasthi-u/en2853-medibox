#ifndef _TEMP_HUMIDITY_H_
#define _TEMP_HUMIDITY_H_

#include "Arduino.h"
#include "DHTesp.h"

#define DHT_PIN                 13

#define HUMIDITY_HIGH           80
#define HUMIDITY_LOW            65
#define TEMPERATURE_HIGH        32
#define TEMPERATURE_LOW         24

extern DHTesp dht;

extern float current_temperature;
extern float current_humidity;

extern String temp_humidity_warning;

void update_temp_humidity();
bool temp_humidity_in_limits();

#endif