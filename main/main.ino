#include <WiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>


// Tentativa de bypass la multiple library de la SD.h
// // Include SD
// #define FS_NO_GLOBALS
// #include <FS.h>
// #ifdef ESP32
//   #include "SPIFFS.h" // ESP32 only
// #endif

#include <SPI.h>
#include <SD.h>

//Wifi
const char* ssid = "SAO";
const char* password = "Zxcv1234";
const char* serverUrl = "http://10.131.89.239/capture"; // IP-ul de la ESP32 CameraWebServer -> ESP32-CAM ruleaza CameraWebServer din Exemple->ESP32->Camera

TFT_eSPI tft = TFT_eSPI();

#define BTN 27
#define SD_MISO 26
#define SD_MOSI 13
#define SD_SCK 14
#define SD_CS 15
int i = 1;

SPIClass spiSD(HSPI);

//Output la ecran
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (y >= tft.height()) return false;
  tft.pushImage(x, y, w, h, bitmap);
  return true;
}

void setup() {
  Serial.begin(115200);

  pinMode(BTN, INPUT_PULLUP);
  
  // Conectare WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");

  // Pentru ST7789
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);

  TJpgDec.setJpgScale(1);
  TJpgDec.setCallback(tft_output);

  tft.setCursor(10, 100);
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  tft.println("Init SD...");

  spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, spiSD)) {
    Serial.println("SD failed");
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 100);
    tft.setTextColor(TFT_RED);
    tft.println("SD FAILED!");
    while (1);
  }
  Serial.println("SD OK");

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
             if(digitalRead(BTN) == LOW)
              {
                Serial.println("Saving image...");
                char filename[32];
                snprintf(filename, sizeof(filename), "/image%d.jpg", i);
                Serial.print("Saving as ");
                Serial.println(filename);
                File file = SD.open(filename, FILE_WRITE);
                if(!file)
                  {
                    Serial.println("Error creating file");
                  }
                else
                  {
                    file.write(buff, len);
                    // file.println("A mers");
                    file.close();
                    Serial.println("Saved!");
                    i = i+1;
                  }
                while(digitalRead(BTN) == LOW) {delay(10);};
              }

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