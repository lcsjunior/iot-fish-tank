#include "espx_wifi.h"

WifiClass Wifi;
IPAddress apIP(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

bool mountFS() {
  delay(1000);
  bool ret;
#if defined(ESP8266)
  ret = LittleFS.begin();
#else
  ret = LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED);
#endif
  if (!ret) {
    Serial.println(F("Failed to mount LittleFS"));
  }
  return ret;
}

time_t now() { return time(nullptr); }

time_t uptime() { return (time_t)(millis() / 1000); }

void getLocalTimeFmt(char *buf, size_t len) {
  time_t now = time(nullptr);
  struct tm *timeinfo;
  timeinfo = localtime(&now);
  strftime(buf, len, DATETIME_FORMAT, timeinfo);
}

void printLocalTime() {
  char buf[32];
  getLocalTimeFmt(buf, sizeof(buf));
  Serial.println(buf);
}

void printMAC(const uint8_t *mac_addr) {
  char macStr[18];
  snprintf_P(macStr, sizeof(macStr), PSTR("%02x:%02x:%02x:%02x:%02x:%02x"),
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4],
             mac_addr[5]);
  Serial.print(macStr);
}

int str2mac(const char *mac, uint8_t *values) {
  if (6 == sscanf(mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &values[0], &values[1],
                  &values[2], &values[3], &values[4], &values[5])) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t dBm2Quality(const int16_t dBm) {
  if (dBm <= -100)
    return 0;
  else if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}

void WifiClass::initAP() {
  char apSsid[32];
  sprintf_P(apSsid, "ESPsoftAP-%06x", getChipId());

  Serial.print(F("Setting soft-AP configuration... "));
  Serial.println(WiFi.softAPConfig(apIP, apIP, subnet) ? F("Ready")
                                                       : F("Failed!"));

  Serial.print(F("Setting soft-AP... "));
  Serial.println(WiFi.softAP(apSsid, _apPass) ? F("Ready") : F("Failed!"));

  Serial.print(F("AP IP Address:      "));
  Serial.println(WiFi.softAPIP());

  Serial.print(F("SSID:               "));
  Serial.println(WiFi.softAPSSID());

  Serial.print(F("AP MAC Address:     "));
  Serial.println(WiFi.softAPmacAddress());

  Serial.print(F("Board MAC Address:  "));
  Serial.println(WiFi.macAddress());

  Serial.print(F("Channel:            "));
  Serial.println(WiFi.channel());
  _apChannel = WiFi.channel();
}

void WifiClass::initSTA() {
#if defined(ESP8266)
  sprintf_P(_hostname, "esp8266-%06x", getChipId());
#else
  sprintf_P(_hostname, "esp32-%06x", getChipId());
#endif
  WiFi.setHostname(_hostname);

  WiFi.begin(_ssid, _pass);
  Serial.print(F("Connecting"));
  unsigned long currentMillis = millis();
  while (!WiFi.isConnected() &&
         (millis() - currentMillis) <= WIFI_CONNECT_TIMEOUT) {
    Serial.print(F("."));
    delay(300);
  }
  _isSTAEnabled = true;

  configTzTime(_tz, _ntpServer);
  currentMillis = millis();
  while ((millis() - currentMillis) <= CONFIG_TZ_DELAY) {
    Serial.print(F("."));
    delay(300);
  }
  Serial.println();
  Serial.print(F("Local Time:         "));
  printLocalTime();

  ArduinoOTA.setHostname((const char *)_hostname);
  ArduinoOTA.setPassword((const char *)_otaPass);
  ArduinoOTA.begin();

  Serial.print(F("IP Address:         "));
  Serial.println(WiFi.localIP());

  Serial.print(F("Hostname:           "));
  Serial.println(WiFi.getHostname());

  Serial.print(F("Board MAC Address:  "));
  Serial.println(WiFi.macAddress());

  Serial.print(F("Wi-Fi Channel:      "));
  Serial.println(WiFi.channel());
  _channel = WiFi.channel();

  Serial.print(F("Signal Strength:    "));
  Serial.print(WiFi.RSSI());
  Serial.print(F(" dBm / "));
  Serial.print(dBm2Quality(WiFi.RSSI()));
  Serial.println(F("%"));
}

void WifiClass::loop() {
  if (_shouldReboot) {
    Serial.println(F("Rebooting..."));
    delay(100);
    ESP.restart();
  }
  if (_isSTAEnabled) {
    if (!WiFi.isConnected() &&
        (millis() - _lastWiFiRetryConnectTime) >= WIFI_CONNECT_TIMEOUT) {
      _lastWiFiRetryConnectTime = millis();
      Serial.println(F("Reconnecting to WiFi..."));
      WiFi.disconnect();
      WiFi.begin(_ssid, _pass);
    }
    ArduinoOTA.handle();
  }
}

uint32_t WifiClass::getChipId() {
  if (_chipId > 0) {
    return _chipId;
  }
#if defined(ESP8266)
  _chipId = ESP.getChipId();
#else
  for (int i = 0; i < 17; i = i + 8) {
    _chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
#endif
  return _chipId;
}

uint8_t WifiClass::getAPChannel() const { return _apChannel; }

uint8_t WifiClass::getChannel() const { return _channel; }

void WifiClass::reboot() { _shouldReboot = true; }
