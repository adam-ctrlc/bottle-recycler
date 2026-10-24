#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include "display.h"

static const int LCD_COLS = 16;
static const int LCD_ROWS = 2;

static hd44780_I2Cexp lcd;  // finds the I2C address and backpack wiring by itself

void displayBegin() {
  // Without a timeout, noise on the I2C wires (e.g. from the servos) can make
  // Wire wait forever and freeze the whole machine. With it, the LCD call just
  // fails and the machine keeps working.
  Wire.setWireTimeout(25000, true);
  pinMode(LED_BUILTIN, OUTPUT);
  displayReset();
}

// If the LCD isn't answering, the machine keeps running without it and the
// Arduino's onboard LED stays on so you know the screen has a problem.
void displayReset() {
  bool ok = (lcd.begin(LCD_COLS, LCD_ROWS) == 0);
  digitalWrite(LED_BUILTIN, ok ? LOW : HIGH);
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
  // LCD stopped answering (loose wire, I2C glitch): start it again once
  if (lcd.clear() != 0) {
    displayReset();
    lcd.clear();
  }
  printCentered(0, line1);
  printCentered(1, line2);
}
