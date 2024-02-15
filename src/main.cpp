#include "cube_messages.h"
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

uint8_t broadcast_address[] = {0xD8, 0xBC, 0x38, 0xE5, 0xA8, 0x38};

MessageLetter message_letter;
esp_now_peer_info_t peer_info;

void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print(status == ESP_NOW_SEND_SUCCESS ? "1" : "0");
  if (status == ESP_NOW_SEND_SUCCESS && ++message_letter.letter > 'Z') {
    message_letter.letter = 'A';
  }
}

void on_data_recv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  MessageNfcId nfcid_data;
  memcpy(&nfcid_data, incomingData, sizeof(nfcid_data));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(nfcid_data.id);
}

void setup() {
  Serial.begin(115200);
 
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(on_data_sent);
  
  memcpy(peer_info.peer_addr, broadcast_address, 6);
  peer_info.channel = 0;  
  peer_info.encrypt = false;
  
  if (esp_now_add_peer(&peer_info) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_recv_cb(on_data_recv);

  message_letter.letter = 'A';
}

void loop() {
  esp_err_t result = ESP_FAIL;
  do {  
    Serial.println(String("sending..." + String(message_letter.letter)));
    result = esp_now_send(broadcast_address, (uint8_t *) &message_letter, sizeof(message_letter));
    if (result == ESP_OK) {
      Serial.println(String("Success: ") + message_letter.letter);
    }
    else {
      Serial.print(".");
    }
  } while (result != ESP_OK);
  delay(1000);
}