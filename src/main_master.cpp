#include <Arduino.h>
#include <common.h>
#include <secrets.h>
#include <ESP8266WebServer.h>
#include <uri/UriBraces.h>

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
  Now.addPeer(SLAVE49, Wifi.getChannel());
  Now.addPeer(SLAVE63, Wifi.getChannel());

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
  Serial.printf_P("Data - id: %d, channel: %d, setpoint: %.1f, hysteresis: "
                  "%.1f, thermostat: %d, "
                  "cTemp: %.1f, isHeaterOn: %d, isLedOn: %d\n",
                  incomingReadings.id, incomingReadings.channel,
                  incomingReadings.setpoint, incomingReadings.hysteresis,
                  incomingReadings.thermostat, incomingReadings.cTemp,
                  incomingReadings.isHeaterOn, incomingReadings.isLedOn);
}

void handleRoot() {
  server.send(200, FPSTR(TEXT_PLAIN), FPSTR("hello from ESP!"));
}

uint8_t *getSlaveFromServer() {
  const int slaveId = server.pathArg(0).toInt();
  return getSlaveById(slaveId);
};

void handleSlaveReboot() {
  outgoingReadings.cmd = REBOOT;
  esp_now_send(getSlaveFromServer(), (uint8_t *)&outgoingReadings,
               sizeof(outgoingReadings));
  server.send(200);
}

void handleSlaveBlink() {
  outgoingReadings.cmd = BLINK;
  esp_now_send(getSlaveFromServer(), (uint8_t *)&outgoingReadings,
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
    esp_now_send(getSlaveFromServer(), (uint8_t *)&outgoingReadings,
                 sizeof(outgoingReadings));
  }
  server.send(200);
}

void handleSlaveLedToggle() {
  outgoingReadings.cmd = TOGGLE_LED;
  esp_now_send(getSlaveFromServer(), (uint8_t *)&outgoingReadings,
               sizeof(outgoingReadings));
  server.send(200);
}