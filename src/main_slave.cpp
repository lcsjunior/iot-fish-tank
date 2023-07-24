#include <Arduino.h>
#include <espx_wifi.h>
#include <esp8266_now.h>
#include <config.h>

#define MAX_CHANNEL 13
#define PAIR_CONNECT_TIMEOUT (MILLIS_PER_SECOND * 5)
#define EVENT_INTERVAL (MILLIS_PER_SECOND * 10)

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
PairingStatus pairingStatus = PAIR_REQUEST;

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

unsigned long previousMillis = 0;
unsigned long lastEventTime = 0;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print(F("Last Packet Send Status: "));
  if (sendStatus == 0) {
    Serial.println(F("Delivery success"));
  } else {
    Serial.println(F("Delivery fail"));
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
    if (pairingData.id == 0) {
      esp_now_add_peer(mac, ESP_NOW_ROLE_COMBO, pairingData.channel, NULL, 0);
      memcpy(broadcastAddressX, mac,
             sizeof(broadcastAddressX)); // Adjust broadcastAddressX
      pairingStatus = PAIR_PAIRED;
      saveConfigFile();
    }
    break;
  }
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

    if (esp_now_init() != 0) {
      Serial.println(F("Error initializing ESP-NOW"));
    }
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);

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
    lastEventTime = millis();
    break;
  case PAIR_PAIRED:
    // Nothing to do here
    break;
  }
  return pairingStatus;
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  mountFS();
  if (!loadConfigFile()) {
    Serial.println(F("Using default config"));
  }
  saveConfigFile();

  // WiFi.mode(WIFI_AP_STA);
  // Wifi.setChannel(9);
  // Wifi.initAP();
  // clean esp now
  // esp_now_deinit();
  // WiFi.mode(WIFI_STA);
  // set WiFi channel
  // wifi_promiscuous_enable(1);
  // wifi_set_channel(9);
  // wifi_promiscuous_enable(0);
  // WiFi.printDiag(Serial);
  // WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println(F("Error initializing ESP-NOW"));
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  // esp_now_add_peer(broadcastAddressX, ESP_NOW_ROLE_COMBO, 9, NULL, 0);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  if (autoPairing() == PAIR_PAIRED) {
    if ((millis() - lastEventTime) >= EVENT_INTERVAL) {
      lastEventTime = millis();
      outgoingReadings.msgType = DATA;
      strcpy(outgoingReadings.msg, ENV_NAME);
      esp_now_send(broadcastAddressX, (uint8_t *)&outgoingReadings,
                   sizeof(outgoingReadings));
    }
  }
  delay(100);
}
