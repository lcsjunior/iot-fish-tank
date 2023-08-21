#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <Arduino.h>
#include "relay.h"
#include "temp_sensor.h"
#include "espx_wifi.h"

#define IDLE_TIMEOUT (MILLIS_PER_SECOND * 180)

enum ThermostatState { IDLE, COOLING, HEATING };

class Thermostat {
private:
  ThermostatState _state = IDLE;
  Relay *_k;
  float _setpoint;
  float _hysteresis;
  float _lowerLimit;
  float _upperLimit;

public:
  Thermostat(Relay *k) : _k(k){};
  ThermostatState getState() const;
  void setState(ThermostatState newState);
  char *getStatus() const;
  void setup(const float setpoint, const float hysteresis,
             const float lowerLimit, const float upperLimit);
  void handleCooler(float cTemp);
  void handleHeater(float cTemp);
};

#endif // THERMOSTAT_H