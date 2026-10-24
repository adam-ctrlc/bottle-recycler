// TCRT5000 reflective sensor that detects the plastic bottle
#pragma once

void sensorBegin();
bool bottleDetected();
bool sensorClearWithin(unsigned long timeout);  // true once nothing is seen for CLEAR_TIME in a row
void waitForSensorClear();                      // waits as long as it takes
