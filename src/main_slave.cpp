#include <Arduino.h>
#include <config.h>
#include <espx_wifi.h>
#include <esp8266_now.h>

#define EVENT_INTERVAL (MILLIS_PER_SECOND * 10)

unsigned long lastEventTime = 0;

typedef struct struct_message {
  MessageType msgType;
  char msg[50];
} struct_message;
struct_message outgoingReadings;
// struct_message incomingReadings;

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
      outgoingReadings.msgType = DATA;
      strcpy(outgoingReadings.msg, ENV_NAME);
      esp_now_send(broadcastAddressX, (uint8_t *)&outgoingReadings,
                   sizeof(outgoingReadings));
    }
  }
  delay(100);
}
