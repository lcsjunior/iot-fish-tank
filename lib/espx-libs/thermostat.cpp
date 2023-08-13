#include "thermostat.h"

unsigned long lastStateExitTime = 0;

void Thermostat::setup(const float setpoint, const float hysteresis,
                       const float lowerLimit, const float upperLimit) {
  _setpoint = setpoint;
  _hysteresis = hysteresis;
  _lowerLimit = lowerLimit;
  _upperLimit = upperLimit;
}

ThermostatState Thermostat::getState() const { return _state; }

void Thermostat::setState(ThermostatState state) {
  lastStateExitTime = millis();
  _state = state;
}

char *Thermostat::getStatus() const {
  switch (_state) {
  case IDLE:
    return (char *)"Idle";
  case COOLING:
    return (char *)"C";
  case HEATING:
    return (char *)"H";
  default:
    return (char *)"Undef";
  }
}

void Thermostat::handleCooler() {}

void Thermostat::handleHeater() {
  if (isnan(_tempSensor->getCTemp()) || _tempSensor->getCTemp() < _lowerLimit ||
      _tempSensor->getCTemp() > _upperLimit) {
    _k->turnOff();
    return;
  }

  switch (_state) {
  case IDLE:
    if ((millis() - lastStateExitTime) >= IDLE_TIMEOUT) {
      setState(COOLING);
    }
    break;
  case COOLING:
    _k->turnOff();
    if (_tempSensor->getCTemp() <= _setpoint) {
      setState(HEATING);
    }
    break;
  case HEATING:
    _k->turnOn();
    if (_tempSensor->getCTemp() >= _setpoint + _hysteresis) {
      setState(IDLE);
    }
  }
}
