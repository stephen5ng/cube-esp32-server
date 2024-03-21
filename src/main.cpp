#include "cube_messages.h"
#include <Arduino.h>
#include <math.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* peers[] = {
  "FCB4674F1CE0",
  "E465B8770340",
  "D8BC38F93930",
  "C4DD578E46C8",
  "D8BC38FDE098"};
MessageLetter message_letter;
esp_now_peer_info_t peer_info;

#define LED 2
uint8_t last_led_state = LOW;

// OLED config
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOG_LINES 4

void log(const String& s) {
  static int i = 0;
  if (i++ % LOG_LINES == 0) {
    display.setCursor(0, 0);
    display.clearDisplay();
  }
  display.println(String(i % 100) + ": " + s);
  display.display();
}

String convert_to_hex_string(const uint8_t* data, int dataSize) {
  String hexString = "";
  for (int i = 0; i < dataSize; i++) {
    char hexChars[3];
    sprintf(hexChars, "%02X", data[i]);
    hexString += hexChars;
  }
  return hexString;
}

void convert_to_hex_bytes(const String& hexString, uint8_t* byteArray, unsigned int len) {
  for (int i = 0; i < min(hexString.length(), len*2); i += 2) {
    String hexPair = hexString.substring(i, i+2);
    byteArray[i / 2] = strtoul(hexPair.c_str(), NULL, 16);
  }
}

void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status != ESP_NOW_SEND_SUCCESS) {
    log("SEND FAIL");
  }
  last_led_state = last_led_state == HIGH ? LOW : HIGH;
  digitalWrite(LED, last_led_state);
}

void on_data_recv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  MessageNfcId nfcid_data;
  memcpy(&nfcid_data, incomingData, sizeof(nfcid_data));
  String mac_address = convert_to_hex_string(mac, ESP_NOW_ETH_ALEN);
  String out = String(mac_address) + ":" + String(nfcid_data.id);
  Serial.println(out);  // Transmit to server
  log(out);
  last_led_state = last_led_state == HIGH ? LOW : HIGH;
  digitalWrite(LED, last_led_state);
}

void setup() {
  Serial.begin(115200);
  Serial.println("setup...");
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  Serial.println(F("SSD1306 allocation passed"));
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.display();
  Serial.println(F("display cleared"));
  
  pinMode(LED, OUTPUT);
 
  WiFi.mode(WIFI_STA);
  log(WiFi.macAddress());
  // sleep(2000);
  if (esp_now_init() != ESP_OK) {
    log("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(on_data_sent);

  peer_info.channel = 0;  
  peer_info.encrypt = false;
  for (int ix = 0; ix < sizeof(peers)/sizeof(peers[0]); ix++) {
    log(String("peer: ") + peers[ix]);
    convert_to_hex_bytes(peers[ix], peer_info.peer_addr, ESP_NOW_ETH_ALEN);
    if (esp_now_add_peer(&peer_info) != ESP_OK){
      log("Failed to add peer");
    }
  }
  
  esp_now_register_recv_cb(on_data_recv);
}

void loop() {
  // Wait for messages from game and relay them to the cubes.
  static String in_buffer = "";
  if (Serial.available() <= 0) {
    return;
  }

  char receivedChar = Serial.read();
  if (receivedChar != '\n' && receivedChar != '\r') {
    in_buffer += receivedChar;
    return;
  }

  if (in_buffer.length() == 0) {
    return;
  }

  log(String("< ") + in_buffer);

  int split_pos = in_buffer.indexOf(':');
  if (split_pos == -1) {
    log("BAD INPUT");
    in_buffer = "";
    return;
  }

  String mac = in_buffer.substring(0, split_pos);
  message_letter.letter = in_buffer.substring(split_pos + 1)[0];
  uint8_t send_address[ESP_NOW_ETH_ALEN];
  convert_to_hex_bytes(mac, send_address, ESP_NOW_ETH_ALEN);

  log(String(message_letter.letter) + ">" + mac);
  last_led_state = last_led_state == HIGH ? LOW : HIGH;
  digitalWrite(LED, last_led_state);

  esp_now_send(send_address, (uint8_t *) &message_letter, sizeof(message_letter));  // ignore return
  in_buffer = "";
}
