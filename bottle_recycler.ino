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

void showReady() {
  showMessage("Insert a bottle", "to get a coin");
}

// Opens the flap until the sensor stops seeing the bottle, up to BOTTLE_TRIES
// times. Returns true if the bottle went in.
bool acceptBottle() {
  for (int attempt = 1; attempt <= BOTTLE_TRIES; attempt++) {
    if (attempt > 1) {
      showMessage("One moment...", "Trying again");
    }
    takeBottle();
    if (sensorClearWithin(BOTTLE_GONE_TIMEOUT)) {
      return true;
    }
  }
  return false;
}

// Takes one bottle in and pays for it. The bottle is marked as pending the
// whole time, so if the power goes off halfway the machine finishes it on
// restart instead of losing the coin.
void handleBottle() {
  setBottlePending(true);
  showMessage("Bottle received!", "Please wait...");

  if (acceptBottle()) {
    showMessage("Your coin is", "coming out...");
    dispenseCoin();  // clears the pending mark once the coin is out
    displayReset();  // a servo move can scramble the screen through a power dip
    showMessage("Thank you!", "Take your coin");
    delay(THANK_YOU_TIME);
  } else {
    // No coin: the bottle never went in, or something is held in front of the sensor
    setBottlePending(false);
    displayReset();
    showMessage("Bottle is stuck", "Please remove it");
    waitForSensorClear();
  }

  showReady();
}

void setup() {
  sensorBegin();
  displayBegin();
  showMessage("Hello!", "Getting ready...");
  servosBegin();

  if (bottlePending()) {
    handleBottle();  // the power went off in the middle of a bottle: finish it
  } else {
    showReady();
  }
}

void loop() {
  if (bottleDetected()) {
    handleBottle();
  }
}
