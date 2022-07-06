/*
 * Title: Sender
 * 
 * Author: Johannes Hettinga
 * Version: V1.0
 * Date: 6/7/2022
 */


/*
   --------------------------------------------------------------------
   |                              Init                                |
   --------------------------------------------------------------------
*/

// Connection
#include <esp_now.h>
#include <WiFi.h>

// OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// SHT31 sensor
#include "Adafruit_SHT31.h"
Adafruit_SHT31 sht31 = Adafruit_SHT31();


// Sent message to server
typedef struct struct_message {
  int id;
  float x;
  float y;
} struct_message;
struct_message myData;

// Temp struct for retry part
typedef struct tstruct_message {
  int id;
  float x;
  float y;
} tstruct_message;
tstruct_message tmyData;

esp_now_peer_info_t peerInfo;

int retries = 0;
int delivery = 0;

/*
   --------------------------------------------------------------------
   |                            Config                                |
   --------------------------------------------------------------------
*/

// Mac-adress of server
uint8_t broadcastAddress[] = {0x30, 0xAE, 0xA4, 0x98, 0xB2, 0xB4};

/*
   --------------------------------------------------------------------
   |                            Functions                             |
   --------------------------------------------------------------------
*/

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  (status == ESP_NOW_SEND_SUCCESS ? delivery = 1 : delivery = 0);
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail" );
}

// Homescreen
void homeScreen(String devID, String batPercentage, String var1, String var2, String devName, String ver)
{
  // Line 1, devID & batPercentage
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(devID);
  drawRightString(batPercentage, 128, 0);

  // Line 2, var1
  display.drawRect(1, 8, 64, 48, WHITE);
  display.setTextSize(5);
  display.setCursor(4, 14);
  display.print(var1);

  // Line 3, var2
  display.drawRect(64, 8, 64, 48, WHITE);
  display.setTextSize(5);
  display.setCursor(67, 14);
  display.print(var2);

  // Line 4, devName & ver
  display.setTextSize(1);
  display.setCursor(0, 57);
  display.print(devName);
  drawRightString(ver, 128, 57);
  display.display();
}

// Text display middle
void drawCentreString(String buf, int x, int y) {   //  Draw a centre string on OLED
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
  display.setCursor(x - w / 2, y);
  display.print(buf);
  //Screen: 128,64 get function to exact middle use: 64,32
}

// Text display right
void drawRightString(String buf, int x, int y)
{
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
  display.setCursor(x - w, y);
  display.print(buf);
}

/*
   ---------------------------------------------------------------------
   |                              Setup                                |
   ---------------------------------------------------------------------
*/

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);// Set device as a Wi-Fi Station

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Init OLED display
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  drawCentreString("Starting", 64, 32);
  display.display();

  if (esp_now_init() != ESP_OK) {// Init ESP-NOW
    // error espnow
    display.clearDisplay();
    drawCentreString("espNOW err", 64, 32);
    display.display();
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) { // Add peer
    // error peer adding
    display.clearDisplay();
    drawCentreString("Peer err", 64, 32);
    display.display();
    return;
  }
  delay(10);
  if (! sht31.begin(0x44)) {// Connect sensor
    // error sensor
    display.clearDisplay();
    drawCentreString("SHT31 err", 64, 32);
    display.display();
    while (1) delay(1);
  }

}

/*
   ---------------------------------------------------------------------
   |                              Loop                                 |
   ---------------------------------------------------------------------
*/

void loop() {
  // Set values to send
  float temp = sht31.readTemperature();
  float humi = sht31.readHumidity();
  myData.id = 1;
  myData.x = temp;
  myData.y = humi;

  tmyData.id = myData.id;
  tmyData.x = myData.x;
  tmyData.y = myData.y;

  // Send message via ESP-NOW
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  if (delivery == 1) {
    homeScreen(("ID: " + String(myData.id)), "100%", String(myData.x, 0), String(myData.y, 0), "Sent!" , "0.9.0");
    retries = 0;
  } else while (delivery == 0) {
      homeScreen(("ID: " + String(tmyData.id)), "100%", String(tmyData.x, 0), String(tmyData.y, 0), ("Retrying: " + String(retries)), "0.9.0");
      esp_now_send(broadcastAddress, (uint8_t *) &tmyData, sizeof(tmyData));
      retries++; 
      delay(5000);          
  }
  delay(15000);
 
}
