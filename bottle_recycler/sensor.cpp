#include <Arduino.h>
#include "config.h"
#include "sensor.h"

void sensorBegin() {
  pinMode(SENSOR_PIN, INPUT);
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

// True if something is still in front of the sensor right now
bool sensorBlocked() {
  return digitalRead(SENSOR_PIN) == SENSOR_DETECTED;
}

// Don't pay twice for the same bottle: wait until the sensor is clear
void waitForSensorClear() {
  while (digitalRead(SENSOR_PIN) == SENSOR_DETECTED) {
    delay(50);
  }
}
