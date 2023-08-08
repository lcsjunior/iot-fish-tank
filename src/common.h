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

enum CommandAction { REBOOT = 1 };

typedef struct struct_message {
  MessageType msgType = DATA;
  CommandAction cmd;
  char msg[50];
} struct_message;

#endif // COMMON_H