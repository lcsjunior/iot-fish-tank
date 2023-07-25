#ifndef ESP8266_NOW_H
#define ESP8266_NOW_H

#include <Arduino.h>
#include <config.h>
#include <espx_wifi.h>
#include <espnow.h>

#define MAX_CHANNEL 13
#define PAIR_CONNECT_TIMEOUT (MILLIS_PER_SECOND * 5)

extern uint8_t broadcastAddressX[6];

enum MessageType {
  PAIRING = 1,
  DATA = 2,
};

enum PairingStatus {
  NOT_PAIRED,
  PAIR_REQUEST,
  PAIR_REQUESTED,
  PAIR_PAIRED,
};

void initESPNOW();
PairingStatus autoPairing();

#endif // ESP8266_NOW_H