#include "Arduino.h"
#include "temp_humidity.h"

float current_temperature = 0;
float current_humidity = 0;

String temp_humidity_warning = "";

void update_temp_humidity() {
  TempAndHumidity readings = dht.getTempAndHumidity();
  current_temperature = readings.temperature;
  current_humidity = readings.humidity;
}

bool temp_humidity_in_limits() {
  if ((current_humidity <= HUMIDITY_HIGH) && (current_humidity >= HUMIDITY_LOW) && (current_temperature >= TEMPERATURE_LOW) && (current_temperature <= TEMPERATURE_HIGH)) {
    temp_humidity_warning = "";
    
    return true;
  } else {
    // If the temperature or humidity is out of limits, display a warning

    bool temp_warning = true;
    
    if ((current_temperature > TEMPERATURE_HIGH) && (temp_humidity_warning.indexOf("Temperature too high") == -1)) { // To make sure we do not keep adding to the warning
      temp_humidity_warning = "Temperature too high! ";
    } else if ((current_temperature < TEMPERATURE_LOW) && (temp_humidity_warning.indexOf("Temperature too low") == -1)) {
      temp_humidity_warning = "Temperature too low! ";
    } else {
      temp_humidity_warning = "";
      temp_warning = false;
    }
    
    if ((current_humidity > HUMIDITY_HIGH) && (temp_humidity_warning.indexOf("Humidity too high") == -1)) {
      if (temp_warning) {
        temp_humidity_warning += "Humidity too high!";
      } else {
        temp_humidity_warning = "Humidity too high!";
      }
    } else if ((current_humidity < HUMIDITY_LOW) && (temp_humidity_warning.indexOf("Humidity too low") == -1)) {
      if (temp_warning) {
        temp_humidity_warning += "Humidity too low!";
      } else {
        temp_humidity_warning = "Humidity too low!";
      }
    }
  
    return false;
  }
}