#ifndef _BUZZER_H_
#define _BUZZER_H_

#include "Arduino.h"

#define BUZZER 32

extern bool buzzer_enabled;
extern long buzzer_start_time;
extern long buzzer_end_time;

void enable_buzzer(long beep_duration);
void disable_buzzer();
void buzzer();

#endif