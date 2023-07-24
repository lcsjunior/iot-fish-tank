#include <Arduino.h>
#include <espx_wifi.h>
#include <esp8266_now.h>
#include <config.h>

uint8_t broadcastAddressX[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

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
PairingStatus pairingStatus = NOT_PAIRED;

typedef struct struct_pairing {
  MessageType msgType;
  uint8_t id;
  uint8_t macAddr[6];
  uint8_t channel;
} struct_pairing;

typedef struct struct_message {
  MessageType msgType;
  char msg[50];
} struct_message;

struct_pairing pairingData;
struct_message outgoingReadings;
struct_message incomingReadings;

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
  memcpy_P(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print(len);
  Serial.print(F(" bytes received from: "));
  printMAC(mac);
  Serial.println();
  uint8_t msgType = incomingData[0];
  switch (msgType) {
  case DATA:
    memcpy_P(&incomingReadings, incomingData, sizeof(incomingReadings));
    Serial.println(incomingReadings.msg);
    break;
  case PAIRING:
    memcpy_P(&pairingData, incomingData, sizeof(pairingData));
    printPairingData();
    if (pairingData.id > 0) {
      pairingData.id = 0; // 0 is server
      pairingData.channel = Wifi.getChannel();
      esp_now_send(mac, (uint8_t *)&pairingData, sizeof(pairingData));
      addPeer(mac, pairingData.channel);
    }
    break;
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  mountFS();
  if (!loadConfigFile()) {
    Serial.println(F("Using default config"));
  }
  saveConfigFile();

  WiFi.mode(WIFI_AP_STA);
  Wifi.initAP();
  Wifi.initSTA();

  if (esp_now_init() != 0) {
    Serial.println(F("Error initializing ESP-NOW"));
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  // esp_now_add_peer(broadcastAddress1, ESP_NOW_ROLE_COMBO, 9, NULL, 0);
  // esp_now_add_peer(broadcastAddress2, ESP_NOW_ROLE_COMBO, 9, NULL, 0);
  // esp_now_add_peer(broadcastAddressX, ESP_NOW_ROLE_COMBO, Wifi.getChannel(),
  //                  NULL, 0);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  Wifi.loop();
  delay(100);
}
