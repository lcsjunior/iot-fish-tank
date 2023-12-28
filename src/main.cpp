#include <Arduino.h>
#include <PubSubClient.h>
#include <CronAlarms.h>
#include <NoDelay.h>
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

noDelay mqttConnectTime(MQTT_CONNECT_TIMEOUT);
noDelay mqttPubTime(MQTT_PUB_TIMEOUT);

unsigned long channelId = CH_ID;

char topic[32];
char msg[255];
char status[64];

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
    snprintf_P(status, sizeof(status), "PUB %s", buf);
    snprintf_P(
        msg, sizeof(msg),
        "field1=%.1f&field2=%.1f&field3=%.1f&field4=%d&field5=%d&status=%s",
        cTemp, config.setpoint, config.hysteresis, heater.isOn(), led.isOn(),
        status);
    mqttPublish();
  }

  delay(300);
}

void mqttConnect() {
  mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);
  mqttSubscribe();
}

void mqttSubscribe() {
  snprintf_P(topic, sizeof(topic), "channels/%ld/subscribe", channelId);
  mqttClient.subscribe(topic);
}

void mqttPublish() {
  snprintf_P(topic, sizeof(topic), "channels/%ld/publish", channelId);
  Serial.print(topic);
  Serial.print(F(" - "));
  Serial.print(msg);
  Serial.print(F(" ("));
  Serial.print(strnlen_P(msg, sizeof(msg)));
  Serial.println(F(" bytes)"));
  mqttClient.publish(topic, msg);
}
