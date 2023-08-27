#include "esp32_now.h"

NowClass Now;
uint8_t broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peer;
uint8_t incomingData_[MAX_MESSAGE_SIZE];

#if SLAVE
PairingStatus pairingStatus = PAIR_REQUEST;
#else
PairingStatus pairingStatus = NOT_PAIRED;
#endif

struct_pairing pairingData;

unsigned long previousMillis = 0;

void printPairingData() {
  printLocalTime();
  Serial.printf_P("Pairing Data - msgType: %d, id: %d, channel: %d\n",
                  pairingData.msgType, pairingData.id, pairingData.channel);
  Serial.print(F("Pairing MAC: "));
  printMAC(pairingData.macAddr);
  Serial.println();
}

void cleanup() {
  esp_now_del_peer(broadcastAddress);
  ESP_ERROR_CHECK(esp_wifi_set_channel(config.channel, WIFI_SECOND_CHAN_NONE));
}

bool NowClass::addPeer(const uint8_t *peer_addr, uint8_t channel) {
  int exists = esp_now_is_peer_exist(peer_addr);
  if (exists) {
    Serial.println(F("Already paired"));
    return true;
  } else {
    memset(&peer, 0, sizeof(esp_now_peer_info_t));
    peer.channel = channel;
    peer.encrypt = false;
    memcpy_P(peer.peer_addr, peer_addr, sizeof(broadcastAddress));
    int addStatus = esp_now_add_peer(&peer);
    if (addStatus == ESP_OK) {
      Serial.println(F("Pair success"));
      return true;
    } else {
      Serial.println(F("Pair failed"));
      return false;
    }
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Serial.print(F("Last Packet Send Status: "));
  // if (status == ESP_NOW_SEND_SUCCESS) {
  //   Serial.println(F("Delivery success"));
  // } else {
  //   Serial.println(F("Delivery fail"));
  // }
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  // Serial.print(len);
  // Serial.print(F(" bytes received from: "));
  // printMAC(mac_addr);
  // Serial.println();
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
      pairingData.channel = Wifi.getAPChannel();
      str2mac(WiFi.softAPmacAddress().c_str(), pairingData.macAddr);

      esp_now_send(mac_addr, (uint8_t *)&pairingData, sizeof(pairingData));
    }
#endif
    break;
  case DATA:
    memcpy_P(&incomingData_, incomingData, sizeof(incomingData_));
    nowDataCallback(incomingData_, len);
    break;
  }
}

void NowClass::initESPNOW() {
  if (esp_now_init() != 0) {
    Serial.println(F("Error initializing ESP-NOW"));
  }
  esp_now_register_send_cb(OnDataSent);
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

    pairingData.id = Wifi.getChipId();
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