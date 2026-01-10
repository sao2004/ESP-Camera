#include <WiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>


// Tentativa de bypass la multiple library de la SD.h
// Include SD
#define FS_NO_GLOBALS
#include <FS.h>
#ifdef ESP32
  #include "SPIFFS.h" // ESP32 only
#endif


//Wifi
const char* ssid = "SAO";
const char* password = "Zxcv1234";
const char* serverUrl = "http://10.131.89.239/capture"; // IP-ul de la ESP32 CameraWebServer -> ESP32-CAM ruleaza CameraWebServer din Exemple->ESP32->Camera

TFT_eSPI tft = TFT_eSPI();

//Output la ecran
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (y >= tft.height()) return false;
  tft.pushImage(x, y, w, h, bitmap);
  return true;
}

void setup() {
  Serial.begin(115200);

  // Conectare WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");

  // Pentru ST7789
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);

  TJpgDec.setJpgScale(1);
  TJpgDec.setCallback(tft_output);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      int len = http.getSize();
      if (len > 0) {
        //Aloc memorie pentru poza
        uint8_t* buff = (uint8_t*)malloc(len);
        if (buff) {
          WiFiClient* stream = http.getStreamPtr();
          int bytes_read = stream->readBytes(buff, len);
          
          if (bytes_read > 0) {
             // Afisare
             TJpgDec.drawJpg(0, 0, buff, len);
          }
          
          free(buff);
        }
      }
    } else {
      Serial.printf("Error HTTP: %d\n", httpCode);
    }
    http.end();
  }
  delay(1);
}
