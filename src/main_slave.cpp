#include <Arduino.h>
#include <common.h>

#define EVENT_INTERVAL (MILLIS_PER_SECOND * 10)

unsigned long lastEventTime = 0;

struct_message outgoingReadings;
struct_message incomingReadings;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  mountFS();
  if (!loadConfigFile()) {
    Serial.println(F("Using default config"));
  }
  saveConfigFile();

  Now.initESPNOW();
}

void loop() {
  Wifi.loop();

  if (Now.autoPairing() == PAIR_PAIRED) {
    if ((millis() - lastEventTime) >= EVENT_INTERVAL) {
      lastEventTime = millis();
      esp_now_send(broadcastAddress, (uint8_t *)&outgoingReadings,
                   sizeof(outgoingReadings));
    }
  }

  delay(100);
}

void callbackData(uint8_t *incomingData, uint8_t len) {
  memcpy_P(&incomingReadings, incomingData, sizeof(incomingReadings));
  switch (incomingReadings.cmd) {
  case CommandAction::REBOOT:
    Wifi.reboot();
    break;
  }
}
