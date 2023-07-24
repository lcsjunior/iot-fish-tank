#ifndef ESPX_WIFI_H
#define ESPX_WIFI_H

#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <ArduinoOTA.h>
#include <LittleFS.h>

#define SERIAL_BAUD_RATE 115200
#define FORMAT_LITTLEFS_IF_FAILED true
#define MILLIS_PER_SECOND 1000UL
#define WIFI_CONNECT_TIMEOUT (MILLIS_PER_SECOND * 60)
#define NTP_SERVER "pool.ntp.org"
#define TZ "<-03>3"

bool mountFS();
time_t now();
time_t uptime();
void printLocalDateTime();
void printMAC(const uint8_t *mac_addr);
uint8_t dBmToQuality(const int16_t dBm);

class WifiClass {
private:
  const char *_ssid = WIFI_SSID;
  const char *_pass = WIFI_PASS;
  const char *_otaPass = OTA_PASS;
  const char *_apPass = AP_PASS;
  const char *_ntpServer = NTP_SERVER;
  const char *_tz = TZ;
  char _hostname[64];
  unsigned long _lastWiFiRetryConnectTime = 0;
  bool _shouldReboot = false;
  bool _isSTAEnabled = false;
  uint32_t _chipId = 0;
  uint8_t _channel = 0;

public:
  WifiClass();
  uint32_t getChipId() const;
  uint8_t getChannel() const;
  void initAP();
  void initSTA();
  void loop();
};
extern WifiClass Wifi;

#endif // ESPX_WIFI_H