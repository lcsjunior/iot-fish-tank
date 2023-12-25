#include "config.h"

Config config;
const char *filename = "/config.json";

bool loadConfigFile() {
  LittleFS.remove(filename);
  File file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.println(F("Failed to open config file"));
    return false;
  }
  DynamicJsonDocument doc(JSON_DOC_SIZE);
  DeserializationError err = deserializeJson(doc, file);
  if (err) {
    Serial.println(F("Failed to deserialize configuration: "));
    Serial.println(err.f_str());
    return false;
  }
  config = doc.as<Config>();
  return true;
}

void saveConfigFile() {
  File file = LittleFS.open(filename, "w");
  if (!file) {
    Serial.println(F("Failed to create config file"));
    return;
  }
  DynamicJsonDocument doc(JSON_DOC_SIZE);
  doc.set(config);
  bool serialized = serializeJsonPretty(doc, file) > 0;
  if (!serialized) {
    Serial.println(F("Failed to serialize configuration"));
  }
}

void printConfigFile() {
  File file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.println(F("Failed to open config file"));
    return;
  }
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();
}

void convertToJson(const Config &src, JsonVariant dst) {
  dst["channel"] = src.channel;
  dst["setpoint"] = src.setpoint;
  dst["hysteresis"] = src.hysteresis;
}

void convertFromJson(JsonVariantConst src, Config &dst) {
  dst.channel = src["channel"];
  dst.setpoint = src["setpoint"];
  dst.hysteresis = src["hysteresis"];
}