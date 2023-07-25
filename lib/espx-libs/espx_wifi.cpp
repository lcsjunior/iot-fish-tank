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

void printLocalDateTime() {
  time_t now = time(nullptr);
  struct tm *timeinfo;
  timeinfo = localtime(&now);
  char buf[64];
  strftime(buf, sizeof(buf), "%A, %B %d %Y %H:%M:%S", timeinfo);
  Serial.print(buf);
}

void printMAC(const uint8_t *mac_addr) {
  char macStr[18];
  snprintf_P(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4],
             mac_addr[5]);
  Serial.print(macStr);
}

uint8_t dBmToQuality(const int16_t dBm) {
  if (dBm <= -100)
    return 0;
  else if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}

WifiClass::WifiClass() {
#if defined(ESP8266)
  _chipId = ESP.getChipId();
#else
  for (int i = 0; i < 17; i = i + 8) {
    _chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
#endif
}

void WifiClass::initAP() {
  char apSsid[32];
  sprintf_P(apSsid, "ESPsoftAP-%06x", _chipId);

  Serial.print(F("Setting soft-AP configuration... "));
  Serial.println(WiFi.softAPConfig(apIP, apIP, subnet) ? F("Ready")
                                                       : F("Failed!"));

  Serial.print(F("Setting soft-AP... "));
  Serial.println(WiFi.softAP(apSsid, _apPass, _channel) ? F("Ready")
                                                        : F("Failed!"));

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
  _channel = WiFi.channel();
}

void WifiClass::initSTA() {
#if defined(ESP8266)
  sprintf_P(_hostname, "esp8266-%06x", _chipId);
#else
  sprintf_P(_hostname, "esp32-%06x", _chipId);
#endif
  WiFi.setHostname(_hostname);
  WiFi.begin(_ssid, _pass);
  Serial.print(F("Connecting"));
  while (!WiFi.isConnected() && millis() <= WIFI_CONNECT_TIMEOUT) {
    Serial.print(F("."));
    delay(500);
  }

  _isSTAEnabled = true;
  configTzTime(_tz, _ntpServer);
  ArduinoOTA.setHostname((const char *)_hostname);
  ArduinoOTA.setPassword((const char *)_otaPass);
  ArduinoOTA.begin();

  Serial.println();
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
  uint8_t dBm = WiFi.RSSI();
  Serial.print(dBm);
  Serial.print(F(" dBm / "));
  Serial.print(dBmToQuality(dBm));
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

uint32_t WifiClass::getChipId() const { return _chipId; }

uint8_t WifiClass::getChannel() const { return _channel; }

void WifiClass::reboot() { _shouldReboot = true; }