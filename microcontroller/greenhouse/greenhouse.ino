#include <Wire.h>
#include <SPI.h>
#include "storage.hpp"
#include "environmentManagement.hpp"
#include "sensing.hpp"

void setup() {
  Serial.begin(115200);
  Wire.begin();
  SPI.begin();
}

void loop() {
  // no need to repeat the melody.
}