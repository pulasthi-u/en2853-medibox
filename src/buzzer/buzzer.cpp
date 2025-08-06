#include "Arduino.h"
#include "buzzer.h"

bool buzzer_enabled = false;
long buzzer_start_time = 0;
long buzzer_end_time = 0;

void enable_buzzer(long beep_duration) {
  buzzer_enabled = true;
  buzzer_start_time = millis();
  buzzer_end_time = millis() + beep_duration;
}

void disable_buzzer() {
  buzzer_enabled = false;
  noTone(BUZZER);
}

void buzzer() {
  if (buzzer_enabled) {
    if (millis() > buzzer_start_time) {
      tone(BUZZER, 262);
      buzzer_end_time = buzzer_start_time + 500;
    }
    if (millis() > buzzer_end_time) {
      noTone(BUZZER);
      buzzer_start_time = buzzer_end_time + 500;
    }
  }
}