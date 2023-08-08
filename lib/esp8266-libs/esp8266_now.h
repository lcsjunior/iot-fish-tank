#ifndef ESP8266_NOW_H
#define ESP8266_NOW_H

#include <Arduino.h>
#include <config.h>
#include <espx_wifi.h>
#include <espnow.h>

#define MAX_NUM_PEER 10
#define MAX_CHANNEL 13
#define PAIR_CONNECT_TIMEOUT (MILLIS_PER_SECOND * 5)

void callbackData(uint8_t *incomingData, uint8_t len);

extern uint8_t broadcastAddress[6];

enum PairingStatus {
  NOT_PAIRED,
  PAIR_REQUEST,
  PAIR_REQUESTED,
  PAIR_PAIRED,
};

enum MessageType { PAIRING = 1, DATA };

typedef struct struct_pairing {
  MessageType msgType = PAIRING;
  uint8_t id;
  uint8_t macAddr[6];
  uint8_t channel;
} struct_pairing;

class NowClass {
public:
  bool addPeer(uint8_t *peer_addr, uint8_t channel);
  void initESPNOW();
  PairingStatus autoPairing();
};
extern NowClass Now;

#endif // ESP8266_NOW_H