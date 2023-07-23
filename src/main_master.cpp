#include <Arduino.h>
#include <espx_wifi.h>
#include <config.h>

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  Serial.print(F("\n["));
  Serial.print(F(ENV_NAME));
  Serial.println(F("]"));

  mountFS();
  if (!loadConfigFile()) {
    Serial.println(F("Using default config"));
  }
  saveConfigFile();

  WiFi.mode(WIFI_AP_STA);
  Wifi.initAP();
  Wifi.initSTA();
}

void loop() {
  Wifi.loop();
  delay(100);
}
