#include "lcd.h"
#include "Arduino.h"
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

void write_to_display(String text, int cursorX, int cursorY, int size, bool highlight) {
  // Size 1 is 5px wide, 8px high. 1px strip between characters. (8th pixel going below the baseline)
  // Other sizes just scale up each of these quantities
  display.setTextSize(size);
  if (highlight) {
    display.setTextColor(BLACK);
    display.fillRect(cursorX, cursorY - 2, 128, 12 * size, WHITE);
  } else {
    display.setTextColor(WHITE);
  }
  display.setCursor(cursorX, cursorY);
  display.println(text);
}

void display_navigation() {
  // Display navigation options
  display.fillTriangle(5, SCREEN_HEIGHT - 5, 9, SCREEN_HEIGHT - 5, 7, SCREEN_HEIGHT - 9, WHITE);
  write_to_display("OK", (SCREEN_WIDTH / 2) - 6, SCREEN_HEIGHT - 12, 1);
  display.fillTriangle(SCREEN_WIDTH - 5, SCREEN_HEIGHT - 9, SCREEN_WIDTH - 9, SCREEN_HEIGHT - 9, SCREEN_WIDTH - 7, SCREEN_HEIGHT - 5, WHITE);
}