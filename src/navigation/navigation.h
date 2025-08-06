#ifndef _NAVIGATION_H_
#define _NAVIGATION_H_

#include "Arduino.h"

#include "../lcd/lcd.h"
#include "../time_alarm/time_alarm.h"
#include "../temp_humidity/temp_humidity.h"

#define MENU                    0
#define TIME_SET                1
#define ALARM_SET               2
#define ALARM_VIEW_DELETE       3
#define HOME                    4
#define ALARM_RING              5
#define ALARM_LIST              6
#define ALARM_EDIT              7
#define ALARM_DATA              8
#define WARNING                 9

typedef struct {
  int pin;
  int state;
} Button;

extern Button ok_btn;
extern Button up_btn;
extern Button down_btn;

extern int current_mode;
extern int next_state;

void home_screen();
void menu();
void time_set();
void alarm_list();
void alarm_set();
void alarm_ring();
void alarm_view();

#endif