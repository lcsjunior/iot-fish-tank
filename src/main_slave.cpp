#include <Arduino.h>
#include <espx_wifi.h>
#include <esp8266_now.h>
#include <config.h>

uint8_t broadcastAddress[6] = {0xDC, 0x4F, 0x22, 0x1C, 0x84, 0xF4};

typedef struct struct_message {
  char msg[50];
} struct_message;

struct_message outgoingReadings;
struct_message incomingReadings;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print(F("Last Packet Send Status: "));
  if (sendStatus == 0) {
    Serial.println(F("Delivery success"));
  } else {
    Serial.println(F("Delivery fail"));
  }
}

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print(len);
  Serial.print(F(" bytes received from: "));
  printMAC(mac);
  Serial.println();
  Serial.println(incomingReadings.msg);
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  Serial.print(F("\n["));
  Serial.print(F(ENV_NAME));
  Serial.println(F("]"));

  mountFS();
  if (!loadConfigFile()) {
    Serial.println(F("Using default config"));
  }
  saveConfigFile();

  WiFi.mode(WIFI_AP_STA);
  Wifi.setChannel(9);
  Wifi.initAP();

  if (esp_now_init() != 0) {
    Serial.println(F("Error initializing ESP-NOW"));
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 9, NULL, 0);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  Wifi.loop();

  strcpy(outgoingReadings.msg, ENV_NAME);
  esp_now_send(broadcastAddress, (uint8_t *)&outgoingReadings,
               sizeof(outgoingReadings));

  delay(5000);
}
