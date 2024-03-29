#include <Arduino.h>
#include <PubSubClient.h>
#include <CronAlarms.h>
#include <NoDelay.h>
#include <ESP8266WebServer.h>
#include <secrets.h>
#include <config.h>
#include <espx_wifi.h>
#include <relay.h>
#include <temp_sensor.h>
#include <thermostat.h>

#define MQTT_CONNECT_TIMEOUT (MILLIS_PER_SECOND * 5) //  5s
#define MQTT_PUB_TIMEOUT (MILLIS_PER_SECOND * 30)    // 30s

WiFiClient espClient;
PubSubClient mqttClient(espClient);
ESP8266WebServer server(80);

noDelay mqttConnectTime(MQTT_CONNECT_TIMEOUT);
noDelay mqttPubTime(MQTT_PUB_TIMEOUT);

unsigned long channelId = CH_ID;

char topic[32];
char msg[255];
char status[96];

const char *cronstr_at_8am = "0 0 8 * * *";
const char *cronstr_at_15pm = "0 0 15 * * *";

Relay led;
Relay heater;
DSTempSensor tempSensor;
Thermostat thermostat(&heater);
float cTemp;

void mqttConnect();
void mqttSubscribe();
void mqttPublish();
void initWebServer();

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  led.setup(D5);
  heater.setup(D6);
  tempSensor.setup(D7);

  mountFS();
  if (!loadConfigFile()) {
    Serial.println(F("Using default config"));
    config.setpoint = 24;
    config.hysteresis = 0.5;
  }
  saveConfigFile();

  thermostat.setup(config.setpoint, config.hysteresis, 0, 30);

  WiFi.mode(WIFI_AP_STA);
  Wifi.initAP();
  Wifi.initSTA();

  mqttClient.setServer(mqttServer, mqttPort);
  mqttConnect();

  Cron.create((char *)cronstr_at_8am, []() { led.turnOn(); }, false);

  Cron.create((char *)cronstr_at_15pm, []() { led.turnOff(); }, false);

  initWebServer();
}

void loop() {
  Wifi.loop();
  Cron.delay();

  cTemp = tempSensor.getCTemp();
  thermostat.handleHeater(cTemp);

  if (!mqttClient.connected() && mqttConnectTime.update()) {
    Serial.println(F("Reconnecting to MQTT..."));
    mqttConnect();
  };
  mqttClient.loop();

  if (mqttPubTime.update()) {
    char buf[64];
    getLocalTimeFmt(buf, sizeof(buf));
    snprintf_P(status, sizeof(status), PSTR("PUB %s RSSI %d dBm (%d pcent)"),
               buf, WiFi.RSSI(), dBm2Quality(WiFi.RSSI()));
    snprintf_P(msg, sizeof(msg),
               PSTR("field1=%.1f&field2=%.1f&field3=%.1f&field4=%d&field5=%d&"
                    "status=%s"),
               cTemp, config.setpoint, config.hysteresis, heater.isOn(),
               led.isOn(), status);
    mqttPublish();
  }

  server.handleClient();

  delay(300);
}

void mqttConnect() {
  mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);
  mqttSubscribe();
}

void mqttSubscribe() {
  snprintf_P(topic, sizeof(topic), PSTR("channels/%ld/subscribe"), channelId);
  mqttClient.subscribe(topic);
}

void mqttPublish() {
  snprintf_P(topic, sizeof(topic), PSTR("channels/%ld/publish"), channelId);
  Serial.print(topic);
  Serial.print(F(" - "));
  Serial.print(msg);
  Serial.print(F(" ("));
  Serial.print(strnlen_P(msg, sizeof(msg)));
  Serial.println(F(" bytes)"));
  mqttClient.publish(topic, msg);
}

void initWebServer() {
  server.on(F("/"), []() {
    server.send(200, FPSTR(TEXT_PLAIN), FPSTR("hello from ESP!"));
  });

  server.on(F("/stats"), []() {
    uint32_t free;
    uint16_t max;
    uint8_t frag;
    ESP.getHeapStats(&free, &max, &frag);
    char buf[64];
    snprintf_P(buf, sizeof(buf), PSTR("free: %7u - max: %7u - frag: %3d%% <- "),
               free, max, frag);
    server.send(200, FPSTR(TEXT_PLAIN), FPSTR(buf));
  });

  server.on(F("/reboot"), []() {
    Wifi.reboot();
    server.send(200);
  });

  server.on(F("/led/toggle"), []() {
    led.toggle();
    server.send(200);
  });

  server.on(F("/msg"), []() {
    char buf[sizeof(msg) + 32];
    size_t len = strnlen_P(msg, sizeof(msg));
    snprintf_P(buf, sizeof(buf), PSTR("%s (%zd bytes)"), msg, len);
    server.send(200, FPSTR(TEXT_PLAIN), FPSTR(buf));
  });

  server.onNotFound(
      []() { server.send(404, FPSTR(TEXT_PLAIN), FPSTR("File not found")); });

  server.begin();
  Serial.println(F("HTTP server started"));
}