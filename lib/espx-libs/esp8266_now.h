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
  DATA,
};

enum PairingStatus {
  NOT_PAIRED,
  PAIR_REQUEST,
  PAIR_REQUESTED,
  PAIR_PAIRED,
};

class NowClass {
public:
  bool addPeer(uint8_t *peer_addr, uint8_t channel);
  void cleanup();
  void initESPNOW();
  PairingStatus autoPairing();
};
extern NowClass Now;

#endif // ESP8266_NOW_H