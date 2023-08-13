#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class TempSensor {
public:
  virtual void setup(const byte pin) = 0;
  virtual float getCTemp() = 0;
};

class DSTempSensor : public TempSensor {
private:
  DallasTemperature _sensors;

public:
  void setup(const byte pin);
  float getCTemp();
};

#endif