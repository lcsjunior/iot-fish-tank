#ifndef WIFI_WRAPPER_H
#define WIFI_WRAPPER_H

#include <ArduinoTrace.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "secrets.h"
#include "datetime_util.h"

#define SERIAL_BAUD_RATE 115200
#define MILLIS_PER_SECOND 1000UL
#define WIFI_CONNECT_TIMEOUT (MILLIS_PER_SECOND * 30)

class WiFiWrapperClass {
private:
  unsigned long _wiFiRetryPreviousMillis = 0;
  bool _shouldReboot = false;
  char _hostname[64];

public:
  static uint32_t getChipId();
  void begin();
  void loop();
};

extern WiFiWrapperClass Wifi;

#endif // WIFI_WRAPPER_H