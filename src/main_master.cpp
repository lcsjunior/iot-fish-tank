#include <Arduino.h>
#include <common.h>
#include <WebServer.h>

uint8_t peersAddress[MAX_NUM_PEER][6] = {PEERS_ADDRS};

struct_message outgoingReadings;
struct_message incomingReadings;

WebServer server(80);

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
  for (uint8_t *peer : peersAddress) {
    Now.addPeer(peer, Wifi.getChannel());
  };

  server.on("/slave/1/reboot", []() {
    outgoingReadings.cmd = CommandAction::REBOOT;
    esp_now_send(peersAddress[1], (uint8_t *)&outgoingReadings,
                 sizeof(outgoingReadings));
    server.send(200);
  });
  server.begin();
  Serial.println(F("HTTP server started"));
}

void loop() {
  Wifi.loop();
  server.handleClient();
  delay(100);
}

void callbackData(uint8_t *incomingData, uint8_t len) {
  memcpy_P(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.println(incomingReadings.id);
}
