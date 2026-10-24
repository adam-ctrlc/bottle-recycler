#include <Arduino.h>
#include <EEPROM.h>
#include <Servo.h>
#include "config.h"
#include "servos.h"

// Hobby servos can't report their angle, so the code remembers each servo's
// angle in EEPROM (kept when the power is off). That way after a power cut or
// a brownout reset it knows where a servo was left and can bring it back to 0.
static const int MAGIC_ADDR        = 0;  // holds MAGIC once this sketch has set up the EEPROM
static const int COIN_ANGLE_ADDR   = 1;
static const int BOTTLE_ANGLE_ADDR = 2;
static const int PENDING_ADDR      = 3;
static const byte MAGIC = 0xB7;

static Servo coinServo;
static Servo bottleServo;

// Moves a servo one degree at a time from where it is now to `to`, so it
// doesn't jerk or pull a current spike.
//
// The saved angle is always the end of the move that's closer to rest. If the
// power goes off mid-move, the servo is somewhere between the two ends, so on
// restart it only ever moves back toward rest. It never swings forward, which
// for the coin servo would push out an extra coin.
static void moveServo(Servo &s, int angleAddr, int rest, int to) {
  int from = s.read();  // last angle sent; still valid after detach()
  if (abs(to - rest) < abs(from - rest)) {
    EEPROM.update(angleAddr, to);  // heading back toward rest: save before moving
  }
  int step = (to > from) ? 1 : -1;
  for (int angle = from; angle != to; angle += step) {
    s.write(angle);
    delay(SERVO_STEP_DELAY);
  }
  s.write(to);
  EEPROM.update(angleAddr, to);  // only writes if the value changed
}

// At power-up, start from the angle saved in EEPROM and sweep slowly back to
// rest instead of snapping there.
static void homeServo(Servo &s, int pin, int angleAddr, int rest) {
  int saved = EEPROM.read(angleAddr);
  if (saved > 180) {
    saved = rest;  // corrupted value
  }
  s.write(saved);
  s.attach(pin);
  moveServo(s, angleAddr, rest, rest);
  delay(SERVO_SETTLE);
  s.detach();
}

void servosBegin() {
  // First run, or EEPROM left over from another sketch: start from scratch so
  // old bytes can't swing the coin servo (and pay a coin) or fake a pending bottle.
  // MAGIC is written last, so a power cut halfway just repeats this next time.
  if (EEPROM.read(MAGIC_ADDR) != MAGIC) {
    EEPROM.update(COIN_ANGLE_ADDR, COIN_REST);
    EEPROM.update(BOTTLE_ANGLE_ADDR, BOTTLE_REST);
    EEPROM.update(PENDING_ADDR, 0);
    EEPROM.update(MAGIC_ADDR, MAGIC);
  }

  // One at a time so they don't both pull current at power-up
  homeServo(coinServo, COIN_SERVO_PIN, COIN_ANGLE_ADDR, COIN_REST);
  homeServo(bottleServo, BOTTLE_SERVO_PIN, BOTTLE_ANGLE_ADDR, BOTTLE_REST);
}

// The servo is switched off (detached) after each move. A detached servo draws
// almost no current and can't jitter, which keeps the 5V line steady for the
// LCD and the sensor.
void takeBottle() {
  bottleServo.attach(BOTTLE_SERVO_PIN);
  moveServo(bottleServo, BOTTLE_ANGLE_ADDR, BOTTLE_REST, BOTTLE_OPEN);
  delay(BOTTLE_HOLD);
  moveServo(bottleServo, BOTTLE_ANGLE_ADDR, BOTTLE_REST, BOTTLE_REST);
  delay(SERVO_SETTLE);
  bottleServo.detach();
}

void dispenseCoin() {
  coinServo.attach(COIN_SERVO_PIN);
  moveServo(coinServo, COIN_ANGLE_ADDR, COIN_REST, COIN_PUSH);
  // The coin is out at the end of the push. Clearing the mark here, not after
  // the return stroke, means a power cut on the way back can't pay twice.
  setBottlePending(false);
  delay(COIN_HOLD);
  moveServo(coinServo, COIN_ANGLE_ADDR, COIN_REST, COIN_REST);
  delay(SERVO_SETTLE);
  coinServo.detach();
}

void setBottlePending(bool pending) {
  EEPROM.update(PENDING_ADDR, pending ? 1 : 0);
}

bool bottlePending() {
  return EEPROM.read(PENDING_ADDR) == 1;
}
