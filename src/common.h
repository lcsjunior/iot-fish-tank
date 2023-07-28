#ifndef COMMON_H
#define COMMON_H

#include <Arduino.h>
#include <esp8266_now.h>

enum CommandAction { REBOOT = 1 };

typedef struct struct_message {
  MessageType msgType = DATA;
  CommandAction cmd;
  char msg[50];
} struct_message;

#endif // COMMON_H