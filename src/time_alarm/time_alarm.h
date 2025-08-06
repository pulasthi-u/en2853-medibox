#ifndef _TIME_ALARM_H_
#define _TIME_ALARM_H_

#include "Arduino.h"
#include "../buzzer/buzzer.h"

#define NUM_ALARMS      2

#define NTP_SERVER      "pool.ntp.org"
#define UTC_OFFSET_DST  0
#define NUM_TIMEZONES   38

#define INACTIVE        0
#define ACTIVE          1
#define RINGING         2
#define SNOOZED         3
#define DISMISSED       4

typedef struct {
  tm time_info;
  int status; 
} Alarm;

extern struct tm time_info;
extern int utc_offset;

extern char time_str[9];

extern Alarm alarms[NUM_ALARMS];

extern String timezones[NUM_TIMEZONES];
extern long timezones_numerical[NUM_TIMEZONES];

void get_time_string(tm* time_info_ptr, bool seconds = true);
void set_alarm_state(int alarm_index, int state);
bool is_alarm_due(int alarm_index);
bool is_alarm_ringing(int alarm_index);

#endif