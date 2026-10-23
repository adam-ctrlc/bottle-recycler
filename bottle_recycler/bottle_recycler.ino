// Plastic bottle -> coin machine
// TCRT5000 detects a bottle, the bottle servo moves it in, then the coin servo
// turns 0 -> 180 -> 0 to push one coin out.
//
// Files:
//   config.h              pins, angles and timings (change settings here)
//   display.h / .cpp      I2C LCD
//   sensor.h / .cpp       TCRT5000 bottle sensor
//   servos.h / .cpp       coin servo and bottle servo
//
// LCD text is kept to 16 letters per line so it always fits the screen.

#include "config.h"
#include "display.h"
#include "sensor.h"
#include "servos.h"

void setup() {
  sensorBegin();
  displayBegin();
  showMessage("Hello!", "Getting ready...");
  servosBegin();
  showReady();
}

void loop() {
  if (!bottleDetected()) {
    return;
  }

  showMessage("Bottle received!", "Please wait...");
  takeBottle();

  showMessage("Your coin is", "coming out...");
  dispenseCoin();

  showMessage("Thank you!", "Take your coin");
  delay(THANK_YOU_TIME);

  // Still blocked after all that means the bottle didn't go in
  if (sensorBlocked()) {
    showMessage("Bottle is stuck", "Please remove it");
    waitForSensorClear();
  }

  showReady();
}

void showReady() {
  showMessage("Insert a bottle", "to get a coin");
}
