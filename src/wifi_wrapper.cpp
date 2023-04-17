#include "wifi_wrapper.h"

static const char *ssid = SECRET_SSID;
static const char *pass = SECRET_PASS;
static const char *otaPass = OTA_PASS;
static const char *myTz = "<-03>3";
static const char *ntpServer = "pool.ntp.org";

uint32_t WiFiWrapperClass::getChipId() { return ESP.getChipId(); }

void WiFiWrapperClass::begin() {
  WiFi.mode(WIFI_STA);
#if defined(ESP8266)
  sprintf(_hostname, "esp8266-%06x", getChipId());
#endif
  WiFi.setHostname(_hostname);
  WiFi.begin(ssid, pass);
  Serial.print(F("Connecting"));
  while (WiFi.status() != WL_CONNECTED && millis() <= WIFI_CONNECT_TIMEOUT) {
    Serial.print(F("."));
    delay(500);
  }
  configTzTime(myTz, ntpServer);
  ArduinoOTA.setHostname((const char *)_hostname);
  ArduinoOTA.setPassword((const char *)otaPass);
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
    WiFi.begin(ssid, pass);
    _wiFiRetryPreviousMillis = currentMillis;
  }
  ArduinoOTA.handle();
}

WiFiWrapperClass Wifi;