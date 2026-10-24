#include <Arduino.h>
#include "config.h"
#include "sensor.h"

void sensorBegin() {
  // Pull-up: if the sensor wire comes loose, the pin reads "nothing there"
  // (with the default SENSOR_DETECTED = LOW) instead of floating and faking bottles.
  pinMode(SENSOR_PIN, INPUT_PULLUP);
}

// True if the sensor sees something for at least MIN_DETECT_TIME.
// Short enough to catch a bottle rolling past quickly, long enough to ignore
// tiny electrical spikes.
bool bottleDetected() {
  if (digitalRead(SENSOR_PIN) != SENSOR_DETECTED) {
    return false;
  }
  unsigned long start = millis();
  while (digitalRead(SENSOR_PIN) == SENSOR_DETECTED) {
    if (millis() - start >= MIN_DETECT_TIME) {
      return true;
    }
  }
  return false;
}

// "Clear" means nothing seen for CLEAR_TIME in a row. A clear plastic bottle
// can flicker the sensor off for a moment while it's still there, and a single
// clear reading would let the same bottle be counted twice.
bool sensorClearWithin(unsigned long timeout) {
  unsigned long start = millis();
  unsigned long clearSince = start;
  while (millis() - start < timeout) {
    if (digitalRead(SENSOR_PIN) == SENSOR_DETECTED) {
      clearSince = millis();
    } else if (millis() - clearSince >= CLEAR_TIME) {
      return true;
    }
  }
  return false;
}

void waitForSensorClear() {
  unsigned long clearSince = millis();
  while (millis() - clearSince < CLEAR_TIME) {
    if (digitalRead(SENSOR_PIN) == SENSOR_DETECTED) {
      clearSince = millis();
    }
  }
}
