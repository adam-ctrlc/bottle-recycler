#pragma once
#include "../hd44780.h"
#include <string>
#include <vector>
#include <string.h>
struct Screen { std::string line[2]; };
extern bool lcdPresent;
extern std::vector<Screen> screens;
class hd44780_I2Cexp : public hd44780 {
 public:
  int row = 0;
  int begin(int, int) { return lcdPresent ? 0 : -1; }
  int clear() { if (!lcdPresent) return -1; screens.push_back(Screen()); return 0; }
  int setCursor(int, int r) { row = r; return lcdPresent ? 0 : -1; }
  size_t print(const char *s) { if (!lcdPresent || screens.empty()) return 0; screens.back().line[row] += s; return strlen(s); }
};
