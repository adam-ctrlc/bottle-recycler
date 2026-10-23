#include <Arduino.h>
#include <EEPROM.h>
#include <Servo.h>
#include "config.h"
#include "servos.h"

// Hobby servos can't report their angle, so the code remembers the last angle
// each servo reached and saves it in EEPROM (kept when the power is off).
// That way it always knows where a servo was left and can bring it back to 0
// slowly, even after a power cut or a brownout reset.
static const int COIN_ANGLE_ADDR   = 0;  // EEPROM byte for the coin servo
static const int BOTTLE_ANGLE_ADDR = 1;  // EEPROM byte for the bottle servo

static Servo coinServo;
static Servo bottleServo;

// Moves a servo one degree at a time from where it is now to `to`, so it
// doesn't jerk or pull a current spike, then saves the new angle.
static void moveServo(Servo &s, int angleAddr, int to) {
  int from = s.read();  // last angle sent; still valid after detach()
  int step = (to > from) ? 1 : -1;
  for (int angle = from; angle != to; angle += step) {
    s.write(angle);
    delay(SERVO_STEP_DELAY);
  }
  s.write(to);
  EEPROM.update(angleAddr, to);  // only writes if the value changed
}

// Switches the servo on, moves to target, holds, moves back to rest, then
// switches it off again. A detached servo draws almost no current and can't
// jitter, which keeps the 5V line steady for the LCD and the sensor.
static void runServo(Servo &s, int pin, int angleAddr, int rest, int target, unsigned long hold) {
  s.attach(pin);
  moveServo(s, angleAddr, target);
  delay(hold);
  moveServo(s, angleAddr, rest);
  delay(SERVO_SETTLE);
  s.detach();
}

// At power-up, start from the angle saved in EEPROM and sweep slowly back to
// rest instead of snapping there.
static void homeServo(Servo &s, int pin, int angleAddr, int rest) {
  int saved = EEPROM.read(angleAddr);
  if (saved > 180) {
    saved = rest;  // nothing saved yet (a fresh EEPROM reads 255)
  }
  s.write(saved);
  s.attach(pin);
  moveServo(s, angleAddr, rest);
  delay(SERVO_SETTLE);
  s.detach();
}

// Home the servos one at a time so they don't both pull current at power-up
void servosBegin() {
  homeServo(coinServo, COIN_SERVO_PIN, COIN_ANGLE_ADDR, COIN_REST);
  homeServo(bottleServo, BOTTLE_SERVO_PIN, BOTTLE_ANGLE_ADDR, BOTTLE_REST);
}

void takeBottle() {
  runServo(bottleServo, BOTTLE_SERVO_PIN, BOTTLE_ANGLE_ADDR, BOTTLE_REST, BOTTLE_OPEN, BOTTLE_HOLD);
}

void dispenseCoin() {
  runServo(coinServo, COIN_SERVO_PIN, COIN_ANGLE_ADDR, COIN_REST, COIN_PUSH, COIN_HOLD);
}
