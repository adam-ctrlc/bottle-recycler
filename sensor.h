// TCRT5000 reflective sensor that detects the plastic bottle
#pragma once

void sensorBegin();
bool bottleDetected();
bool sensorBlocked();
void waitForSensorClear();
