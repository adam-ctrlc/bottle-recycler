// Servo 1 (coin dispenser) and servo 2 (bottle flap)
#pragma once

void servosBegin();   // brings both servos back to 0 from wherever they were left
void takeBottle();    // servo 2: opens the flap, holds, closes
void dispenseCoin();  // servo 1: 0 -> 180 -> 0 to push one coin out

// A bottle is "pending" from the moment it's detected until its coin is out
// (or it's rejected). Kept in EEPROM, so after a power cut the machine can
// finish that bottle instead of losing the coin.
void setBottlePending(bool pending);
bool bottlePending();
