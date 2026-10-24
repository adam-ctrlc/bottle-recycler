#pragma once
#include <stdint.h>
struct TwoWire { void setWireTimeout(uint32_t, bool) {} void begin() {} };
extern TwoWire Wire;
