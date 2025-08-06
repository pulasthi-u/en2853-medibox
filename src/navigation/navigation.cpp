#include "Arduino.h"

#include "navigation.h"
#include "../lcd/lcd.h"
#include "../time_alarm/time_alarm.h"
#include "../temp_humidity/temp_humidity.h"

int current_mode = HOME;
int next_state = TIME_SET;

void home_screen() {
  // This is what the user sees by default on the screen.
  // Time, temperature, and humidity are displayed.
  // User can open a menu by pressing the OK button.

  // Display the time on top
  get_time_string(&time_info);
  write_to_display(time_str, (SCREEN_WIDTH / 2) - ((8 * 10 + 7 * 2) / 2), 0, 2);

  // Display navigation options
  write_to_display("MENU", (SCREEN_WIDTH / 2) - ((4 * 5 + 3) / 2), SCREEN_HEIGHT - 9, 1);

  // Display the temperature and humidity, and any warnings
  write_to_display("Temperature: " + String(current_temperature, 1) + "C", 1, 18, 1);
  write_to_display("Humidity: " + String(current_humidity, 1) + "%", 1, 28, 1);
  write_to_display(temp_humidity_warning, 1, 38, 1);

  // Take action based on buttons
  if (ok_btn.state == 1) {
    current_mode = MENU;
    next_state = TIME_SET;
  }
}

void menu() {
  // Display the main menu

  int offset_next_state = next_state - TIME_SET;

  // Display the menu options
  write_to_display("1. Set Time", 1, 1, 1, offset_next_state == 0);
  write_to_display("2. Set Alarm", 1, 11, 1, offset_next_state == 1);
  write_to_display("3. View/Delete Alarm", 1, 21, 1, offset_next_state == 2);
  write_to_display("4. Exit Menu", 1, 31, 1, offset_next_state == 3);

  display_navigation(); // Display menu navigation buttons

  // Navigate the menu with buttons, and select menu option with OK
  if (up_btn.state == 1) {
    offset_next_state = (offset_next_state + 1) % 4;
  } else if (down_btn.state == 1) {
    offset_next_state -= 1;
    if (offset_next_state >= 0) {
      offset_next_state = offset_next_state % 4;
    } else {
      offset_next_state = 3;
    }
  } else if (ok_btn.state == 1) {
    current_mode = next_state;
    next_state = 0;
  }

  next_state = offset_next_state + TIME_SET;
}

void time_set() {
  // Select the timezone

  write_to_display("Choose Time Zone", 1, 1, 1);
  write_to_display("UTC" + timezones[next_state], 1, 12, 1);

  // Display operations
  write_to_display("-", 5, SCREEN_HEIGHT - 10, 1);
  write_to_display("+", SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10, 1);
  write_to_display("DONE", (SCREEN_WIDTH / 2) - ((4 * 5 + 3) / 2), SCREEN_HEIGHT - 10, 1);
  
  // Take action based on buttons
  if (up_btn.state == 1) {
    next_state = (next_state + 1) % NUM_TIMEZONES;
  } else if (down_btn.state == 1) {
    next_state = next_state - 1;
    if (next_state >= 0) {
      next_state = next_state % NUM_TIMEZONES;
    } else {
      next_state = NUM_TIMEZONES - 1;
    }
  } else if (ok_btn.state == 1) {
    configTime(utc_offset, UTC_OFFSET_DST, NTP_SERVER);
    next_state = TIME_SET;
    current_mode = MENU;

    // display.clearDisplay();
    // write_to_display("TIMEZONE SET!", (SCREEN_WIDTH / 2) - ((13 * 5 + 12) / 2), 20, 1);
    // display.display();
    // delay(500);
    
    return;
  }
}

void alarm_list() {
  // Display a list of the alarms
  
  int prev_screen = next_state % 10;
  next_state = (next_state - prev_screen) / 10;

  // Display the alarm options
  write_to_display("1. Alarm 1", 1, 1, 1, next_state == 0);
  write_to_display("2. Alarm 2", 1, 11, 1, next_state == 1);
  write_to_display("3. Back", 1, 21, 1, next_state == 2);
  
  display_navigation(); // Display navigation buttons

  // Handle button presses
  if (up_btn.state == 1) {
    next_state = (next_state + 1) % 3;
  } else if (down_btn.state == 1) {
    next_state = next_state - 1;
    if (next_state >= 0) {
      next_state = next_state % 3;
    } else {
      next_state = 2;
    }
  } else if (ok_btn.state == 1) {
    if (next_state == 2) {
      if (prev_screen == ALARM_SET) {
        next_state = 1 + TIME_SET;
      } else if (prev_screen == ALARM_VIEW_DELETE) {
        next_state = 2 + TIME_SET;
      }
      current_mode = MENU;
    } else {
      if (prev_screen == ALARM_SET) {
        // next_state is the alarm index
        next_state = alarms[next_state].time_info.tm_hour * 10 + next_state;
        current_mode = ALARM_EDIT;
      } else if (prev_screen == ALARM_VIEW_DELETE) {
        // Leave next_state as is, inside ALARM_DATA, we will use that to decide which alarm to view/delete
        current_mode = ALARM_DATA;
      }
    }
    return;
  }

  next_state = (next_state * 10) + prev_screen; // Store the current mode and the alarm number in next_state
}

void alarm_set() {
  // next_state is 0 to 23 for hours
  // next_state is 24 to 24 + 60 for minutes
  
  int alarm_index = next_state % 10;
  next_state = (next_state - alarm_index) / 10;
  
  // Display the time.
  get_time_string(&alarms[alarm_index].time_info, false);
  write_to_display(time_str, (SCREEN_WIDTH / 2) - ((5 * 10 + 4 * 2) / 2), 1, 2);

  // Indicate functionality of the buttons graphically.
  write_to_display("-", 5, SCREEN_HEIGHT - 10, 1);
  write_to_display("+", SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10, 1);

  // Take action based on buttons:
 
  // Change from hours to minutes and finalize.
  if ((ok_btn.state == 1) && (next_state <= 23) && (next_state >= 0)) {
    next_state = alarms[alarm_index].time_info.tm_min + 24; // Display next_state - 24 as the minute value
  } else if ((ok_btn.state == 1) && (next_state >= 24) && (next_state <= 83)) {
    
    // display.clearDisplay();
    // write_to_display("ALARM SET!", (SCREEN_WIDTH / 2) - ((10 * 5 + 9) / 2), 20, 1);
    // display.display();
    // delay(500);
    alarms[alarm_index].status = ACTIVE;

    next_state = ALARM_SET;
    current_mode = ALARM_LIST;
    return;
  }

  if ((next_state <= 23) && (next_state >= 0)) { // Set the hour
    write_to_display("Set Hour", (SCREEN_WIDTH / 2) - (8 * 5 + 7) / 2, SCREEN_HEIGHT - 25, 1);
    write_to_display("NEXT", (SCREEN_WIDTH / 2) - ((4 * 5 + 3) / 2), SCREEN_HEIGHT - 10, 1);

    if (up_btn.state == HIGH) {
      next_state = (next_state + 1) % 24;
    } else if (down_btn.state == HIGH) {
      next_state = next_state - 1;
      if (next_state >= 0) {
        next_state = next_state % 24;
      } else {
        next_state = 23;
      }
    }
    
    alarms[alarm_index].time_info.tm_hour = next_state;

  } else if ((next_state <= 83) && (next_state >= 24)) { // Set the minute
    write_to_display("Set Minute", (SCREEN_WIDTH / 2) - (10 * 5 + 9) / 2, SCREEN_HEIGHT - 25, 1);
    write_to_display("DONE", (SCREEN_WIDTH / 2) - ((4 * 5 + 3) / 2), SCREEN_HEIGHT - 10, 1);

    int minute = next_state - 24;
    if (up_btn.state == HIGH) {
      minute = (minute + 1) % 60;
    } else if (down_btn.state == HIGH) {
      minute = minute - 1;
      if (minute >= 0) {
        minute = minute % 60;
      } else {
        minute = 59;
      }
    }
    next_state = 24 + minute;

    alarms[alarm_index].time_info.tm_min = minute;
  }

  next_state = (next_state * 10) + alarm_index;
}

void alarm_ring() {
  // Screen that shows when the alarm is ringing

  int alarm_index = next_state % 10;
  int prev_screen = ((next_state - alarm_index) / 10) % 10;
  int existing_next_state = (((next_state - alarm_index) / 10) - prev_screen) / 10;

  write_to_display("MEDICINE TIME!", (SCREEN_WIDTH / 2) - 5 * 7 - 7, 10, 1);

  // Display options to user to snooze or dismiss
  write_to_display("Snooze", 1, SCREEN_HEIGHT - 9, 1);
  write_to_display("Dismiss", SCREEN_WIDTH - 5 * 7 - 7, SCREEN_HEIGHT - 9, 1);

  // Take action based on buttons
  if (up_btn.state == 1) {
    // Dismiss
    set_alarm_state(alarm_index, DISMISSED);

    current_mode = prev_screen;
    next_state = existing_next_state;
  } else if (down_btn.state == 1) {
    // Snooze
    set_alarm_state(alarm_index, SNOOZED);

    current_mode = prev_screen;
    next_state = existing_next_state;
  }
}

void alarm_view() {
  // Show the time and status of a particular alarm, with the option to delete

  // next_state is the alarm index
  if (alarms[next_state].status == INACTIVE) { // Alarm has been deleted or not yet been set
    // Indicate that the alarm is not set. User should only be able to go back.
    write_to_display("Alarm not set.", 1, 1, 1);
    write_to_display("Go Back", 1, SCREEN_HEIGHT - 9, 1);

    // Go back based on button press
    if (down_btn.state == 1) {
      current_mode = ALARM_LIST;
      next_state = next_state * 10 + ALARM_VIEW_DELETE;
    }
  } else {
    if (alarms[next_state].status == ACTIVE) { // Alarm is set to ring sometime in future 
      write_to_display("Alarm set to ring at", 1, 1, 1);
    } else if (alarms[next_state].status == SNOOZED) { // Alarm rang earlier, but user snoozed it, so it is not inactive
      write_to_display("Alarm snoozed for", 1, 1, 1);
    }else if (alarms[next_state].status == DISMISSED) { // Alarm rang earlier, and user dismissed it
      write_to_display("Alarm dismissed at", 1, 1, 1);
    }

    // Show the time of the alarm
    String time_set = String(alarms[next_state].time_info.tm_hour) + ":" + String(alarms[next_state].time_info.tm_min);
    write_to_display(time_set, 1, 11, 2);

    // Show option to go back and to delete the alarm.
    write_to_display("Go Back", 1, SCREEN_HEIGHT - 9, 1);
    write_to_display("Delete", SCREEN_WIDTH - 5 * 6 - 6, SCREEN_HEIGHT - 9, 1);

    // Take action based on buttons
    if (up_btn.state == 1) {
      // Delete
      set_alarm_state(next_state, INACTIVE);

      // display.clearDisplay();
      // write_to_display("ALARM DELETED!", (SCREEN_WIDTH / 2) - ((14 * 5 + 13) / 2), 20, 1);
      // display.display();
      // delay(500);

      current_mode = ALARM_DATA;
    } else if (down_btn.state == 1) {
      // Back
      current_mode = ALARM_LIST;
      next_state = next_state * 10 + ALARM_VIEW_DELETE;
    }
  }
}