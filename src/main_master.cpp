#include <Arduino.h>
#include <config.h>
#include <common.h>
#include <secrets.h>
#include <espx_wifi.h>
#include <esp8266_now.h>

int incomingCmd;

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

void handleSerialCommand() {
  switch (incomingCmd) {
  case CommandAction::REBOOT:
    outgoingReadings.cmd = CommandAction::REBOOT;
    esp_now_send(peerAddress[1], (uint8_t *)&outgoingReadings,
                 sizeof(outgoingReadings));
    break;
  default:
    break;
  }
}

void loop() {
  Wifi.loop();

  if (Serial.available() > 0) {
    incomingCmd = Serial.parseInt();
    handleSerialCommand();
  }

  delay(100);
}

void callbackData(uint8_t *incomingData, uint8_t len) {
  memcpy_P(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.println(incomingReadings.msg);
}
