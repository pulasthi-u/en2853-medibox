#include "Arduino.h"

#include "time_alarm.h"
#include "../buzzer/buzzer.h"

String timezones[NUM_TIMEZONES] = {
  "-12:00", "-11:00", "-10:30", "-09:30", "-09:00", "-08:00", "-07:00", "-06:00", "-05:00", "-04:00", "-03:30", "-03:00", "-02:00",
  "-01:00", "+00:00", "+01:00", "+02:00", "+03:00", "+03:30", "+04:00", "+04:30", "+05:00", "+05:30", "+05:45", "+06:00", "+06:30",
  "+07:00", "+08:00", "+08:45", "+09:00", "+09:30", "+10:00", "+10:30", "+11:00", "+12:00", "+12:45", "+13:00", "+14:00"
}; // We hardcode these values because timezones are not regularly spaced

long timezones_numerical[NUM_TIMEZONES] = {
  -43200, -39600, -37800, -34200, -32400, -28800, -25200, -21600, -18000, -14400, -12600, -10800, -7200,
  -3600, 0, 3600, 7200, 10800, 12600, 14400, 16200, 18000, 19800, 20700, 21600, 23400,
  25200, 28800, 31500, 32400, 34200, 36000, 37800, 39600, 43200, 45900, 46800, 50400
};

int utc_offset = 19800;
struct tm time_info;
char time_str[9]; // HH:MM:SS

void get_time_string(tm* time_info_ptr, bool seconds) {
  // If seconds is set, we display the time in HH:MM:SS format, otherwise as HH:MM
  if (seconds) {
    strftime(time_str, sizeof(time_str), "%H:%M:%S", time_info_ptr);
  } else {
    strftime(time_str, sizeof(time_str), "%H:%M", time_info_ptr);
  }
}

void set_alarm_state(int alarm_index, int state) {
  // Set the alarm state to the given state, and update the relevant variables
  if (state == RINGING) {
    // Alarm is ringing, user has to snooze or dismiss. Buzzer must be enabled.
    alarms[alarm_index].status = RINGING;
    
    enable_buzzer(500);
  } else if (state == SNOOZED) {
    // User asked to snooze the alarm. Update alarm time to 5 minutes later, and disable buzzer.
    alarms[alarm_index].status = SNOOZED;

    unsigned int alarm_time_seconds = alarms[alarm_index].time_info.tm_hour * 3600 + alarms[alarm_index].time_info.tm_min * 60;
    unsigned int snooze_time_seconds = alarm_time_seconds + 300; // Snooze for 5 minutes
 
    alarms[alarm_index].time_info.tm_sec = snooze_time_seconds % 60;
    snooze_time_seconds = (snooze_time_seconds - alarms[alarm_index].time_info.tm_sec) / 60;

    alarms[alarm_index].time_info.tm_min = snooze_time_seconds % 60;
    snooze_time_seconds = (snooze_time_seconds - alarms[alarm_index].time_info.tm_min) / 60;

    alarms[alarm_index].time_info.tm_hour = snooze_time_seconds % 24;
    
    disable_buzzer();
  } else if (state == DISMISSED) {
    // User dismissed the alarm. Disable buzzer.
    alarms[alarm_index].status = DISMISSED;
    
    disable_buzzer();
  } else if (state == INACTIVE) {
    // Alarm has not been set, or deleted by the user.
    alarms[alarm_index].status = INACTIVE;
    alarms[alarm_index].time_info.tm_hour = 0;
    alarms[alarm_index].time_info.tm_min = 0;
  }
}

bool is_alarm_due(int alarm_index) {
  Alarm alarm = alarms[alarm_index];

  // Only active/snoozed alarms that are not yet ringing can be due
  if (((alarm.status == ACTIVE) || (alarm.status == SNOOZED)) && (alarm.status != RINGING)) {
      struct tm alarm_time_info = alarm.time_info;
      
      // Because we are not storing the date of the alarm there can be certain issues, when its close to around midnight IG
      long time_now_seconds = time_info.tm_hour * 3600 + time_info.tm_min * 60 + time_info.tm_sec;
      long alarm_time_seconds = alarm_time_info.tm_hour * 3600 + alarm_time_info.tm_min * 60;
      
      if (time_now_seconds >= alarm_time_seconds) {
        return true;
      }
  }

  return false;
}

bool is_alarm_ringing(int alarm_index) {
  if (alarms[alarm_index].status == RINGING) {
    return true;
  }
  return false;
}