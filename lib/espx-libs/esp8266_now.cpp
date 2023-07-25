#include "esp8266_now.h"

uint8_t broadcastAddressX[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#if SLAVE
PairingStatus pairingStatus = PAIR_REQUEST;
#else
PairingStatus pairingStatus = NOT_PAIRED;
#endif

typedef struct struct_pairing {
  MessageType msgType;
  uint8_t id;
  uint8_t macAddr[6];
  uint8_t channel;
} struct_pairing;
struct_pairing pairingData;

unsigned long previousMillis = 0;

void printPairingData() {
  printLocalDateTime();
  Serial.printf_P("\nPairing: msgType: %d, id: %d, channel: %d\n",
                  pairingData.msgType, pairingData.id, pairingData.channel);
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print(F("Last Packet Send Status: "));
  if (sendStatus == 0) {
    Serial.println(F("Delivery success"));
  } else {
    Serial.println(F("Delivery fail"));
  }
}

bool addPeer(uint8_t *peer_addr, uint8_t channel) {
  int exists = esp_now_is_peer_exist(peer_addr);
  if (exists) {
    Serial.println(F("Already Paired"));
    return true;
  } else {
    int addStatus =
        esp_now_add_peer(peer_addr, ESP_NOW_ROLE_COMBO, channel, NULL, 0);
    if (addStatus == 0) {
      Serial.println(F("Pair success"));
      return true;
    } else {
      Serial.println(F("Pair failed"));
      return false;
    }
  }
}

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  Serial.print(len);
  Serial.print(F(" bytes received from: "));
  printMAC(mac);
  Serial.println();
  uint8_t msgType = incomingData[0];
  switch (msgType) {
  case DATA:
    break;
  case PAIRING:
    memcpy_P(&pairingData, incomingData, sizeof(pairingData));
    printPairingData();
#if SLAVE
    if (pairingData.id == 0) {
      esp_now_add_peer(mac, ESP_NOW_ROLE_COMBO, pairingData.channel, NULL, 0);
      memcpy(broadcastAddressX, mac,
             sizeof(broadcastAddressX)); // Adjust broadcastAddressX
      pairingStatus = PAIR_PAIRED;
      saveConfigFile();
    }
#else
    if (pairingData.id > 0) {
      addPeer(mac, pairingData.channel);
      pairingData.id = 0; // 0 is server
      pairingData.channel = Wifi.getChannel();
      esp_now_send(mac, (uint8_t *)&pairingData, sizeof(pairingData));
    }
#endif
    break;
  }
}

void initESPNOW() {
  if (esp_now_init() != 0) {
    Serial.println(F("Error initializing ESP-NOW"));
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
}

PairingStatus autoPairing() {
  switch (pairingStatus) {
  case NOT_PAIRED:
    // Nothing to do here
    break;
  case PAIR_REQUEST:
    Serial.print(F("Pairing request on channel "));
    Serial.println(config.channel);

    // Clean esp now
    esp_now_deinit();
    WiFi.mode(WIFI_STA);
    wifi_promiscuous_enable(1);
    wifi_set_channel(config.channel);
    wifi_promiscuous_enable(0);
    WiFi.disconnect();
    initESPNOW();

    pairingData.msgType = PAIRING;
    pairingData.id = BOARD_ID;
    pairingData.channel = config.channel;
    esp_now_send(broadcastAddressX, (uint8_t *)&pairingData,
                 sizeof(pairingData));
    previousMillis = millis();
    pairingStatus = PAIR_REQUESTED;
    break;
  case PAIR_REQUESTED:
    if ((millis() - previousMillis) >= PAIR_CONNECT_TIMEOUT) {
      previousMillis = millis();
      config.channel++;
      if (config.channel > MAX_CHANNEL) {
        config.channel = 0;
      }
      pairingStatus = PAIR_REQUEST;
    }
    break;
  case PAIR_PAIRED:
    // Nothing to do here
    break;
  }
  return pairingStatus;
}