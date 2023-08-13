#include "esp8266_now.h"

NowClass Now;
uint8_t broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#if SLAVE
PairingStatus pairingStatus = PAIR_REQUEST;
#else
PairingStatus pairingStatus = NOT_PAIRED;
#endif

struct_pairing pairingData;

unsigned long previousMillis = 0;

void printPairingData() {
  printLocalDateTime();
  Serial.printf_P("\nPairing Data:  msgType: %d, id: %d, channel: %d\n",
                  pairingData.msgType, pairingData.id, pairingData.channel);
  Serial.print(F("Pairing MAC: "));
  printMAC(pairingData.macAddr);
  Serial.println();
}

void cleanup() {
  esp_now_deinit();
  WiFi.mode(WIFI_STA);
  wifi_promiscuous_enable(1);
  wifi_set_channel(config.channel);
  wifi_promiscuous_enable(0);
  WiFi.disconnect();
}

bool NowClass::addPeer(uint8_t *peer_addr, uint8_t channel) {
  int exists = esp_now_is_peer_exist(peer_addr);
  if (exists) {
    Serial.println(F("Already paired"));
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

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print(F("Last Packet Send Status: "));
  if (sendStatus == 0) {
    Serial.println(F("Delivery success"));
  } else {
    Serial.println(F("Delivery fail"));
  }
}

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
#if DEBUG
  Serial.print(len);
  Serial.print(F(" bytes received from: "));
  printMAC(mac);
  Serial.println();
#endif
  uint8_t msgType = incomingData[0];
  switch (msgType) {
  case PAIRING:
    memcpy_P(&pairingData, incomingData, sizeof(pairingData));
    printPairingData();
#if SLAVE
    if (pairingData.id == 0) {
      Now.addPeer(pairingData.macAddr, pairingData.channel);

      memcpy(broadcastAddress, pairingData.macAddr,
             sizeof(broadcastAddress)); // Adjust broadcastAddress
      pairingStatus = PAIR_PAIRED;
      saveConfigFile();
    }
#else
    if (pairingData.id > 0) {
      Now.addPeer(pairingData.macAddr, pairingData.channel);

      pairingData.id = 0; // 0 is server
      pairingData.channel = Wifi.getChannel();
      str2mac(WiFi.macAddress().c_str(), pairingData.macAddr);

      esp_now_send(mac, (uint8_t *)&pairingData, sizeof(pairingData));
    }
#endif
    break;
  case DATA:
    callbackData(incomingData, len);
    break;
  }
}

void NowClass::initESPNOW() {
  if (esp_now_init() != 0) {
    Serial.println(F("Error initializing ESP-NOW"));
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
#if DEBUG
  esp_now_register_send_cb(OnDataSent);
#endif
  esp_now_register_recv_cb(OnDataRecv);
}

PairingStatus NowClass::autoPairing() {
  switch (pairingStatus) {
  case NOT_PAIRED:
    // Nothing to do here
    break;
  case PAIR_REQUEST:
    Serial.print(F("Pairing request on channel "));
    Serial.println(config.channel);
    cleanup();
    initESPNOW();

    pairingData.id = BOARD_ID;
    pairingData.channel = config.channel;
    str2mac(WiFi.macAddress().c_str(), pairingData.macAddr);

    esp_now_send(broadcastAddress, (uint8_t *)&pairingData,
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