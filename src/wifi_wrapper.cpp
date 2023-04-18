#include "wifi_wrapper.h"

uint32_t WiFiWrapperClass::getChipId() const { return ESP.getChipId(); }

void WiFiWrapperClass::begin() {
  WiFi.mode(WIFI_STA);
#if defined(ESP8266)
  sprintf(_hostname, "esp8266-%06x", getChipId());
#endif
  WiFi.setHostname(_hostname);
  WiFi.begin(_ssid, _pass);
  Serial.print(F("Connecting"));
  while (WiFi.status() != WL_CONNECTED && millis() <= WIFI_CONNECT_TIMEOUT) {
    Serial.print(F("."));
    delay(500);
  }
  configTzTime(_myTz, _ntpServer);
  ArduinoOTA.setHostname((const char *)_hostname);
  ArduinoOTA.setPassword((const char *)_otaPass);
  ArduinoOTA.begin();
}

void WiFiWrapperClass::loop() {
  unsigned long currentMillis = millis();
  if (_shouldReboot) {
    Serial.println(F("Rebooting..."));
    delay(100);
    ESP.restart();
  }
  if (WiFi.status() != WL_CONNECTED &&
      currentMillis - _wiFiRetryPreviousMillis >= WIFI_CONNECT_TIMEOUT) {
    Serial.println(F("Reconnecting to WiFi..."));
    WiFi.disconnect();
    WiFi.begin(_ssid, _pass);
    _wiFiRetryPreviousMillis = currentMillis;
  }
  ArduinoOTA.handle();
}

WiFiWrapperClass Wifi;