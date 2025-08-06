#ifndef _LCD_H_
#define _LCD_H_

#include "Arduino.h"
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64

extern Adafruit_SSD1306 display;

void write_to_display(String text, int cursorX, int cursorY, int size, bool highlight = false);
void display_navigation();

#endif