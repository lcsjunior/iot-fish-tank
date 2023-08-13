#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <Arduino.h>
#include "relay.h"
#include "temp_sensor.h"
#include "espx_wifi.h"

#define IDLE_TIMEOUT (MILLIS_PER_SECOND * 60)

enum ThermostatState { IDLE, COOLING, HEATING };

class Thermostat {
private:
  ThermostatState _state = IDLE;
  TempSensor *_tempSensor;
  Relay *_k;
  float _setpoint;
  float _hysteresis;
  float _lowerLimit;
  float _upperLimit;

public:
  Thermostat(TempSensor *tempSensor, Relay *k)
      : _tempSensor(tempSensor), _k(k){};
  ThermostatState getState() const;
  void setState(ThermostatState newState);
  char *getStatus() const;
  void setup(const float setpoint, const float hysteresis,
             const float lowerLimit, const float upperLimit);
  void handleCooler();
  void handleHeater();
};

#endif // THERMOSTAT_H