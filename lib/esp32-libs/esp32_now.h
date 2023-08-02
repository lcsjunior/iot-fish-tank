#ifndef ESP32_NOW_H
#define ESP32_NOW_H

#include <Arduino.h>
#include <config.h>
#include <espx_wifi.h>
#include <esp_now.h>

#define MAX_NUM_PEER 10
#define MAX_CHANNEL 13
#define MAX_MESSAGE_SIZE 255
#define PAIR_CONNECT_TIMEOUT (MILLIS_PER_SECOND * 5)

void callbackData(uint8_t *incomingData, uint8_t len);

extern uint8_t broadcastAddressX[6];

enum PairingStatus {
  NOT_PAIRED,
  PAIR_REQUEST,
  PAIR_REQUESTED,
  PAIR_PAIRED,
};

enum MessageType { PAIRING = 1, DATA };

class NowClass {
public:
  bool addPeer(const uint8_t *peer_addr, uint8_t channel);
  void initESPNOW();
  PairingStatus autoPairing();
};
extern NowClass Now;

#endif // ESP32_NOW_H