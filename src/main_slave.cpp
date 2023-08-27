#include <Arduino.h>
#include <common.h>
#include <relay.h>
#include <temp_sensor.h>

#define EVENT_INTERVAL (MILLIS_PER_SECOND * 30)

unsigned long lastEventTime = 0;

struct_message outgoingReadings;
struct_message incomingReadings;

Relay led;
Relay heater;
DSTempSensor tempSensor;
Thermostat thermostat(&heater);
float cTemp;

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
    config.channel = 0;
    config.setpoint = 25;
    config.hysteresis = 0.5;
  }
  saveConfigFile();
  printConfigFile();

  Now.initESPNOW();

  thermostat.setup(config.setpoint, config.hysteresis, 0, 30);
}

void loop() {
  Wifi.loop();

  cTemp = tempSensor.getCTemp();
  thermostat.handleHeater(cTemp);

  if (Now.autoPairing() == PAIR_PAIRED) {
    if ((millis() - lastEventTime) >= EVENT_INTERVAL) {
      lastEventTime = millis();
      outgoingReadings.cmd = NONE;
      outgoingReadings.channel = config.channel;
      outgoingReadings.setpoint = config.setpoint;
      outgoingReadings.hysteresis = config.hysteresis;
      outgoingReadings.thermostat = thermostat.getState();
      outgoingReadings.cTemp = cTemp;
      outgoingReadings.isHeaterOn = heater.isOn();
      outgoingReadings.isLedOn = led.isOn();
      esp_now_send(broadcastAddress, (uint8_t *)&outgoingReadings,
                   sizeof(outgoingReadings));
    }
  }

  if (incomingReadings.cmd == BLINK) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);
    incomingReadings.cmd = NONE;
  }

  delay(300);
}

void callbackData(uint8_t *incomingData, uint8_t len) {
  memcpy_P(&incomingReadings, incomingData, sizeof(incomingReadings));
  switch (incomingReadings.cmd) {
  case NONE:
    break;
  case REBOOT:
    Wifi.reboot();
    break;
  case BLINK:
    break;
  case SET_PREFS:
    config.channel = incomingReadings.channel;
    config.setpoint = incomingReadings.setpoint;
    config.hysteresis = incomingReadings.hysteresis;
    saveConfigFile();
    break;
  case TOGGLE_LED:
    led.toggle();
    break;
  case TURN_ON_LED:
    led.turnOn();
    break;
  case TURN_OFF_LED:
    led.turnOff();
    break;
  }
}
