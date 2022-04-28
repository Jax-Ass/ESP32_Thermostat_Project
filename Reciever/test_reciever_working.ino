// Reciever

#include <esp_now.h>
#include <WiFi.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int id;
  float x;
  float y;
}struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a structure to hold the readings from each board
struct_message board1;
//struct_message board2;
//struct_message board3;

// Create an array with all the structures
struct_message boardsStruct[1] = {board1};

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Update the structures with the new incoming data
  boardsStruct[myData.id-1].x = myData.x;
  boardsStruct[myData.id-1].y = myData.y;
  Serial.printf("x value: %d \n", boardsStruct[myData.id-1].x);
  Serial.printf("y value: %d \n", boardsStruct[myData.id-1].y);
  Serial.println();
}

void drawCentreString(String buf, int x, int y) {   //  Draw a centre string on OLED
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
  display.setCursor(x - w / 2, y);
  display.print(buf);
}

void drawRightString(String buf, int x, int y)  { //  Draw a right string on OLED
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
  display.setCursor(x - w, y);
  display.print(buf);
}
 
void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);
  
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // OLED setup:
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
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
  
  float board1X = boardsStruct[0].x;
  float board1Y = boardsStruct[0].y;
  float board2X = boardsStruct[1].x;
  float board2Y = boardsStruct[1].y;
  //int board3X = boardsStruct[2].x;
  //int board3Y = boardsStruct[2].y;
  Serial.println(board1X);
  Serial.println(board1Y);
  Serial.println(board2X);
  Serial.println(board2Y);
  display.clearDisplay();
  drawCentreString(String(board1X, 1), 64, 16);
  drawCentreString(String(board1Y, 1), 64, 32);
  display.display();
}
