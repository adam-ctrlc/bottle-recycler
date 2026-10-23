// All pins and settings in one place. Change values here, not in the other files.
#pragma once
#include <Arduino.h>

// ---- Pins ----
const int COIN_SERVO_PIN   = 2;  // servo 1: coin dispenser
const int BOTTLE_SERVO_PIN = 3;  // servo 2: bottle flap/pusher
const int SENSOR_PIN       = 4;  // TCRT5000 module "D0" output
// LCD: SDA -> Uno SDA, SCL -> Uno SCL (pins next to AREF; address is found automatically)

// Most TCRT5000 modules pull D0 LOW when something is in front of the sensor.
// If yours works the other way round, change this to HIGH.
const int SENSOR_DETECTED = LOW;

// ---- Servo angles ----
const int COIN_REST   = 0;
const int COIN_PUSH   = 180;
const int BOTTLE_REST = 0;
const int BOTTLE_OPEN = 90;

// ---- Timing (ms) ----
const unsigned long MIN_DETECT_TIME  = 5;    // a bottle only has to be seen this long, so fast ones still count
const unsigned long BOTTLE_HOLD      = 1000; // time the bottle flap stays open
const unsigned long COIN_HOLD        = 500;  // time the coin servo waits at 180
const unsigned long SERVO_STEP_DELAY = 10;   // per degree; slower = less current = less brownout
const unsigned long SERVO_SETTLE     = 300;  // let the servo finish moving before switching it off
const unsigned long THANK_YOU_TIME   = 2000; // how long "Thank you!" stays on screen
