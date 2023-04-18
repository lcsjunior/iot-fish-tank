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
#define WIFI_CONNECT_TIMEOUT (MILLIS_PER_SECOND * 30)

class WiFiWrapperClass {
private:
  const char *_ssid = SECRET_SSID;
  const char *_pass = SECRET_PASS;
  const char *_otaPass = OTA_PASS;
  const char *_myTz = "<-03>3";
  const char *_ntpServer = "pool.ntp.org";
  char _hostname[64];
  unsigned long _wiFiRetryPreviousMillis = 0;
  bool _shouldReboot = false;

public:
  uint32_t getChipId() const;
  void begin();
  void loop();
};

extern WiFiWrapperClass Wifi;

#endif // WIFI_WRAPPER_H