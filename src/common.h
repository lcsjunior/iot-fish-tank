#ifndef COMMON_H
#define COMMON_H

#include <Arduino.h>
#include <config.h>
#include <espx_wifi.h>
#if defined(ESP8266)
#include <esp8266_now.h>
#else
#include <esp32_now.h>
#endif

enum Command { NONE = 1, REBOOT, BLINK, SET_PREFS, TOGGLE_LED };

typedef struct struct_message {
  MessageType msgType = DATA;
  uint8_t id = BOARD_ID;
  Command cmd = NONE;
  uint8_t channel;
  float setpoint;
  float hysteresis;
  float cTemp;
  uint8_t isHeaterOn;
  uint8_t isLedOn;
} struct_message;

#endif // COMMON_H