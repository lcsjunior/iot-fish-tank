#include <Arduino.h>
#include <common.h>
#include <ESP8266WebServer.h>
#include <uri/UriBraces.h>

#define NUM_PEER 3

uint8_t peersAddress[NUM_PEER][6] = {PEERS_ADDRS};

struct_message outgoingReadings;
struct_message incomingReadings;

ESP8266WebServer server(80);

void handleRoot();
void handleSlaveReboot();
void handleSlaveBlink();
void handleSlavePrefs();
void handleSlaveLedToggle();

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

  server.on(F("/"), handleRoot);
  server.on(UriBraces(F("/slaves/{}/reboot")), handleSlaveReboot);
  server.on(UriBraces(F("/slaves/{}/blink")), handleSlaveBlink);
  server.on(UriBraces(F("/slaves/{}/prefs")), handleSlavePrefs);
  server.on(UriBraces(F("/slaves/{}/led/toggle")), handleSlaveLedToggle);
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
  Serial.printf_P(
      "Data - id: %d, channel: %d, setpoint: %.1f, hysteresis: %.1f, "
      "cTemp: %.1f, isHeaterOn: %d, isLedOn: %d\n",
      incomingReadings.id, incomingReadings.channel, incomingReadings.setpoint,
      incomingReadings.hysteresis, incomingReadings.cTemp,
      incomingReadings.isHeaterOn, incomingReadings.isLedOn);
}

void handleRoot() {
  server.send(200, FPSTR(TEXT_PLAIN), FPSTR("hello from ESP!"));
}

uint8_t getSlaveId() { return server.pathArg(0).toInt(); }

void handleSlaveReboot() {
  outgoingReadings.cmd = REBOOT;
  esp_now_send(peersAddress[getSlaveId()], (uint8_t *)&outgoingReadings,
               sizeof(outgoingReadings));
  server.send(200);
}

void handleSlaveBlink() {
  outgoingReadings.cmd = BLINK;
  esp_now_send(peersAddress[getSlaveId()], (uint8_t *)&outgoingReadings,
               sizeof(outgoingReadings));
  server.send(200);
}

void handleSlavePrefs() {
  if (server.method() == HTTP_POST) {
    for (uint8_t i = 0; i < server.args(); i++) {
      Serial.printf_P(server.argName(i).c_str());
      Serial.printf_P(": ");
      Serial.printf_P(server.arg(i).c_str());
      Serial.printf_P("    ");
    };
    Serial.println();
    outgoingReadings.cmd = SET_PREFS;
    outgoingReadings.channel = server.arg(F("channel")).toInt();
    outgoingReadings.setpoint = server.arg(F("setpoint")).toFloat();
    outgoingReadings.hysteresis = server.arg(F("hysteresis")).toFloat();
    esp_now_send(peersAddress[getSlaveId()], (uint8_t *)&outgoingReadings,
                 sizeof(outgoingReadings));
  }
  server.send(200);
}

void handleSlaveLedToggle() {
  outgoingReadings.cmd = TOGGLE_LED;
  esp_now_send(peersAddress[getSlaveId()], (uint8_t *)&outgoingReadings,
               sizeof(outgoingReadings));
  server.send(200);
}