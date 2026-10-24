#pragma once
#include <stdint.h>
struct EEPROMClass {
  uint8_t mem[1024];
  uint8_t read(int a) { return mem[a]; }
  void update(int a, uint8_t v);
};
extern EEPROMClass EEPROM;
