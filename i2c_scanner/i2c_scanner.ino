// I2C scanner: prints the address of every I2C device found on SDA/SCL.
// Open the Serial Monitor at 9600 baud. Rescans every 5 seconds.

#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(9600);
  while (!Serial) {}  // needed on boards with native USB (Leonardo, Micro)
  Serial.println("I2C scanner");
}

void loop() {
  int found = 0;
  Serial.println("Scanning...");

  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("  Device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      found++;
    } else if (error == 4) {
      Serial.print("  Unknown error at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  if (found == 0) {
    Serial.println("  No I2C devices found - check SDA, SCL, VCC and GND");
  } else {
    Serial.print("  Done, ");
    Serial.print(found);
    Serial.println(" device(s) found");
  }
  Serial.println();
  delay(5000);
}
