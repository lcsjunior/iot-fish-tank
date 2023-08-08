#include <Arduino.h>
#include <common.h>

uint8_t peersAddress[MAX_NUM_PEER][6] = {PEERS_ADDRS};

struct_message outgoingReadings;
struct_message incomingReadings;

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
  Now.initESPNOW();
}

void loop() {
  Wifi.loop();
  delay(100);
}

void callbackData(uint8_t *incomingData, uint8_t len) {
  memcpy_P(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.println(incomingReadings.msg);
}
