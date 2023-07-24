#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#define JSON_DOC_SIZE 1024

struct Config {
  uint8_t channel = 0;
};
extern Config config;

bool loadConfigFile();
void saveConfigFile();
void printConfigFile();

#endif // CONFIG_H