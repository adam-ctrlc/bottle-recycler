// 16x2 I2C LCD (hd44780 library)
#pragma once

void displayBegin();
void displayReset();  // starts the LCD again; fixes a screen scrambled by a power dip
void showMessage(const char *line1, const char *line2 = "");  // centered, max 16 letters per line
