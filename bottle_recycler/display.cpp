#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include "display.h"

static const int LCD_COLS = 16;
static const int LCD_ROWS = 2;

static hd44780_I2Cexp lcd;  // finds the I2C address and backpack wiring by itself

void displayBegin() {
  int status = lcd.begin(LCD_COLS, LCD_ROWS);
  if (status) {
    // LCD not found: blinks the onboard LED (pin 13) with the error code
    hd44780::fatalError(status);
  }
}

static void printCentered(int row, const char *text) {
  int pad = (LCD_COLS - (int)strlen(text)) / 2;
  if (pad < 0) {
    pad = 0;
  }
  lcd.setCursor(pad, row);
  lcd.print(text);
}

void showMessage(const char *line1, const char *line2) {
  lcd.clear();
  printCentered(0, line1);
  printCentered(1, line2);
}
