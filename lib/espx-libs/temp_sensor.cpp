#include "temp_sensor.h"

void DSTempSensor::setup(const byte pin) {
  _sensors = DallasTemperature(new OneWire(pin));
  _sensors.begin();
}

float DSTempSensor::getCTemp() {
  _sensors.requestTemperatures();
  float cTemp = _sensors.getTempCByIndex(0);
  if (cTemp == DEVICE_DISCONNECTED_C) {
    Serial.println(F("Error: Could not read temperature data"));
  }
  return cTemp;
}
