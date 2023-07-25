#include <Arduino.h>
#include <config.h>
#include <espx_wifi.h>
#include <esp8266_now.h>

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  mountFS();
  if (!loadConfigFile()) {
    Serial.println(F("Using default config"));
  }
  saveConfigFile();

  WiFi.mode(WIFI_AP_STA);
  Wifi.initAP();
  Wifi.initSTA();
  initESPNOW();
}

void loop() {
  Wifi.loop();
  delay(100);
}
