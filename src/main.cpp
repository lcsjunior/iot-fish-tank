#include <Arduino.h>
#include "wifi_wrapper.h"

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Wifi.begin();
}

void loop() {
  Wifi.loop();
  delay(300);
}