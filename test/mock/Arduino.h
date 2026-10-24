#pragma once
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
typedef uint8_t byte;
#define LOW 0
#define HIGH 1
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define LED_BUILTIN 13
unsigned long millis();
void delay(unsigned long ms);
int digitalRead(int pin);
void pinMode(int pin, int mode);
void digitalWrite(int pin, int val);
