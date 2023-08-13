#include "temp_sensor.h"

void DSTempSensor::setup(const byte pin) {
  _sensors = DallasTemperature(new OneWire(pin));
  _sensors.begin();
}

float DSTempSensor::getCTemp() {
  _sensors.requestTemperatures();
  return _sensors.getTempCByIndex(0);
}
