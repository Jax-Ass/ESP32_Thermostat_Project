/*
   Title: Reciever

   Author: Johannes Hettinga
   Version: V1
   Date: 17/5/2022
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

// SD
#include "FS.h"
#include "SD.h"
#include <SPI.h>
#define SD_CS 5

// Recieving message from client
typedef struct struct_message {
  int id;
  float x;
  float y;
} struct_message;
struct_message myData;

/*
   --------------------------------------------------------------------
   |                            Config                                |
   --------------------------------------------------------------------
*/

// Boards
struct_message board1;
//struct_message board2;
//struct_message board3;

// Create an array with all the structures
struct_message boardsStruct[1] = {board1};

float tboard1X;
float tboard1Y;
float tboard2X;
float tboard2Y;
int tID;

/*
   --------------------------------------------------------------------
   |                            Functions                             |
   --------------------------------------------------------------------
*/

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  tID = myData.id;
  // Update the structures with the new incoming data
  boardsStruct[myData.id - 1].x = myData.x;
  boardsStruct[myData.id - 1].y = myData.y;
  Serial.printf("x value: %d \n", boardsStruct[myData.id - 1].x);
  Serial.printf("y value: %d \n", boardsStruct[myData.id - 1].y);
  Serial.println();
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

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

/*
   ---------------------------------------------------------------------
   |                              Setup                                |
   ---------------------------------------------------------------------
*/

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  drawCentreString("Starting", 64, 32);
  display.display();


  // Initialize SD card
  SD.begin(SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;    // init failed
  }
  File file = SD.open("/data.csv");
  if (!file) {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/data.csv", "\r\n");
  }
  else {
    Serial.println("File already exists");
  }
  file.close();


  esp_now_register_recv_cb(OnDataRecv);
}

/*
   ---------------------------------------------------------------------
   |                              Loop                                 |
   ---------------------------------------------------------------------
*/

void loop() {

  float board1X = boardsStruct[0].x;
  float board1Y = boardsStruct[0].y;
  float board2X = boardsStruct[1].x;
  float board2Y = boardsStruct[1].y;


  if(tboard1X != board1X ||
     tboard1Y != board1Y ||
     tboard2X != board2X ||
     tboard2Y != board2Y) {
    String msg("Board 1," + String(board1X) + "," + String(board1Y) + "\r\n");

  File file = SD.open("/data.csv");
  if (file) {
    appendFile(SD, "/data.csv", msg.c_str());
  }
  file.close();

  float temp[] = {board1X, board2X};
  float humi[] = {board1Y, board2Y};
  int nr = tID -1;
  
  Serial.println(tID);
  Serial.println(nr);
  Serial.println(temp[nr]);
  Serial.println(humi[nr]);

  display.clearDisplay();
  drawCentreString(("ID: " + tID), 64, 0);
  drawCentreString(String(temp[nr], 1), 64, 16);
  drawCentreString(String(humi[nr], 1), 64, 32);
  display.display();
  
  } 

  

  tboard1X = board1X;
  tboard1Y = board1Y;
  tboard2X = board2X;
  tboard2Y = board2Y;
}
