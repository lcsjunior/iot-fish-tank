#include <Arduino.h>
#include <common.h>
#include <secrets.h>
#include <CronAlarms.h>
#include <ESP8266WebServer.h>
#include <uri/UriBraces.h>
#include <PubSubClient.h>
#include <NoDelay.h>

#define MQTT_TOPIC_BUFFER_SIZE 64
#define MQTT_MSG_BUFFER_SIZE 128

uint8_t peer49[6] = PEER49;
uint8_t peer63[6] = PEER63;

struct_message outgoingReadings;
struct_message incomingReadings;

const char *cronstr_at_8am = "0 0 8 * * *";
const char *cronstr_at_15pm = "0 0 15 * * *";

void nowSend(const uint8_t *peer);
void nowSendBroadcast();

WiFiClient espClient;
PubSubClient mqttClient(espClient);

noDelay mqttRetryConnectTime(MILLIS_PER_SECOND * 5);

char topic[MQTT_TOPIC_BUFFER_SIZE];
char msg[MQTT_MSG_BUFFER_SIZE];

void mqttSubscriptionCallback(char *topic, byte *payload, unsigned int length);
void mqttConnect();
void mqttSubscribe(long subChannelId);
void mqttPublish(long pubChannelId);

ESP8266WebServer server(80);

void handleRoot();
void handlePeerReboot();
void handlePeerBlink();
void handlePeerPrefs();
void handlePeerLedToggle();

long getChannelFromIncReadings();
uint8_t *getPeerFromServer();

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
  Now.addPeer((uint8_t *)peer49, Wifi.getChannel());
  Now.addPeer((uint8_t *)peer63, Wifi.getChannel());

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqttSubscriptionCallback);
  mqttConnect();

  Cron.create((char *)cronstr_at_8am,
              []() {
                outgoingReadings.cmd = TURN_ON_LED;
                nowSendBroadcast();
              },
              false);

  Cron.create((char *)cronstr_at_15pm,
              []() {
                outgoingReadings.cmd = TURN_OFF_LED;
                nowSendBroadcast();
              },
              false);

  server.on(F("/"), handleRoot);
  server.on(UriBraces(F("/peers/{}/reboot")), handlePeerReboot);
  server.on(UriBraces(F("/peers/{}/blink")), handlePeerBlink);
  server.on(UriBraces(F("/peers/{}/prefs")), handlePeerPrefs);
  server.on(UriBraces(F("/peers/{}/led/toggle")), handlePeerLedToggle);
  server.begin();
  Serial.println(F("HTTP server started"));
}

void loop() {
  Wifi.loop();
  Cron.delay();

  if (!mqttClient.connected() && mqttRetryConnectTime.update()) {
    Serial.println(F("Reconnecting to MQTT..."));
    mqttConnect();
  };
  mqttClient.loop();

  server.handleClient();

  delay(300);
}

void nowSend(uint8_t *peer) {
  esp_now_send(peer, (uint8_t *)&outgoingReadings, sizeof(outgoingReadings));
}

void nowSendBroadcast() {
  nowSend(peer49);
  nowSend(peer63);
}

void mqttConnect() {
  mqttClient.connect(SECRET_MQTT_CLIENT_ID, SECRET_MQTT_USERNAME,
                     SECRET_MQTT_PASSWORD);
  mqttSubscribe(PEER49_CH_ID);
  mqttSubscribe(PEER63_CH_ID);
}

void mqttSubscribe(long subChannelId) {
  snprintf_P(topic, MQTT_TOPIC_BUFFER_SIZE, "channels/%ld/subscribe",
             subChannelId);
  mqttClient.subscribe(topic);
}

void mqttPublish(long pubChannelId) {
  snprintf_P(topic, MQTT_TOPIC_BUFFER_SIZE, "channels/%ld/publish",
             pubChannelId);
  // Serial.print(topic);
  // Serial.print(F("    "));
  // Serial.println(msg);
  mqttClient.publish(topic, msg);
}

void nowDataCallback(uint8_t *incomingData, uint8_t len) {
  memcpy_P(&incomingReadings, incomingData, sizeof(incomingReadings));

  // Serial.printf_P("Data - id: %d, channel: %d, setpoint: %.1f, hysteresis: "
  //                 "%.1f, thermostat: %d, "
  //                 "cTemp: %.1f, isHeaterOn: %d, isLedOn: %d \n",
  //                 incomingReadings.id, incomingReadings.channel,
  //                 incomingReadings.setpoint, incomingReadings.hysteresis,
  //                 incomingReadings.thermostat, incomingReadings.cTemp,
  //                 incomingReadings.isHeaterOn, incomingReadings.isLedOn);

  snprintf_P(msg, MQTT_MSG_BUFFER_SIZE,
             "field1=%.1f&field2=%.1f&field3=%.1f&field4=%d&field5=%d",
             incomingReadings.cTemp, incomingReadings.setpoint,
             incomingReadings.hysteresis, incomingReadings.isHeaterOn,
             incomingReadings.isLedOn);
  mqttPublish(getChannelFromIncReadings());
}

void mqttSubscriptionCallback(char *topic, byte *payload, unsigned int length) {
  // Serial.print(F("Message arrived ["));
  // Serial.print(topic);
  // Serial.print(F("] "));
  // for (int i = 0; i < (int)length; i++) {
  //   Serial.print((char)payload[i]);
  // }
  // Serial.println();
}

void handleRoot() {
  server.send(200, FPSTR(TEXT_PLAIN), FPSTR("hello from ESP!"));
}

void handlePeerReboot() {
  outgoingReadings.cmd = REBOOT;
  nowSend(getPeerFromServer());
  server.send(200);
}

void handlePeerBlink() {
  outgoingReadings.cmd = BLINK;
  nowSend(getPeerFromServer());
  server.send(200);
}

void handlePeerPrefs() {
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
    nowSend(getPeerFromServer());
  }
  server.send(200);
}

void handlePeerLedToggle() {
  outgoingReadings.cmd = TOGGLE_LED;
  nowSend(getPeerFromServer());
  server.send(200);
}

long getChannelFromIncReadings() {
  if (incomingReadings.id == PEER49_ID) {
    return PEER49_CH_ID;
  } else if (incomingReadings.id == PEER63_ID) {
    return PEER63_CH_ID;
  }
  return -1;
}

uint8_t *getPeerFromServer() {
  const int peerId = server.pathArg(0).toInt();
  if (peerId == PEER49_ID) {
    return peer49;
  } else if (peerId == PEER63_ID) {
    return peer63;
  }
  return nullptr;
};
