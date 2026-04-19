/*
 Developers: HipMonsters.com
 Creation Date: Apr 2 2026
 License: MIT

 Wearable Robot Monitor P

 Screen: GC9A01 ROUND LCD DISPLAY 
 Arduino Nanao ESP32
 
 Pinout:  
  CS  >   D10
  DC  >   D9
  RES >   D8
  SDA >   D11
  SCL >   D13
  VCC >   3.3V
  GND >   GND 
 */
 
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"
#include <ArduinoJson.h>

#define tft_cs 10
#define tft_dc  9
#define tft_rst 8

Adafruit_GC9A01A tft(tft_cs,tft_dc,tft_rst); 

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// UUIDs for the BLE Service and Characteristics
#define SERVICE_UUID "rth1209e-45u1-8724-b7f5-ea07361b19c7"
// For reading data
#define CHARACTERISTIC_UUID_RX "rth1209e-45u1-8724-b7f5-ea07361b19c7" 
// For pushing data
#define CHARACTERISTIC_UUID_TX "rth1209e-45u1-8724-b7f5-ea07361b19c8" 

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;

bool deviceConnected = false;
bool dataUpdated = true;

uint8_t txValue = 0;
// Callback for connection events
class MyServerCallbacks: public BLEServerCallbacks {
 void onConnect(BLEServer* pServer) {
   deviceConnected = true;
 }
 void onDisconnect(BLEServer* pServer) {
   deviceConnected = false;

   pServer->getAdvertising()->start();
 }
};

String updateMessage1 = "MeasureA  0";
String updateMessage2 = "MeasureB  0";
String updateMessage3 = "MeasureC  0";
String updateMessage4 = "MeasureD  0"; 
String updateMessage5 = " ?????  "; 

// Callback for handling data written to the RX characteristic

class MyCallbacks: public BLECharacteristicCallbacks {
 
  void onRead(BLECharacteristic* pCharacteristic) override {

    Serial.println("Client requested to read the characteristic.");
 
    // Example: dynamically generate a value
    static int counter = 0;
    counter++;

    // Convert counter to string and set as value
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "Count: %d", counter);
 
    pCharacteristic->setValue(buffer);

    Serial.print("Sent value: ");
    Serial.println(buffer);
  }
 
 
 void onWrite(BLECharacteristic *pCharacteristic) {
   std::string rxValue = pCharacteristic->getValue();
   if (rxValue.length() > 0) {  

     //updateMessage = rxValue.c_str(); 
     JsonDocument doc;
     deserializeJson(doc, rxValue.c_str());

     String value = doc["MeasureA"];  
     updateMessage1 = "MeasureA  " +  value ;
     
     String value2 = doc["MeasureB"];  
     updateMessage2 = "MeasureB   " +  value2  ;
     
     String value3 = doc["MeasureC"];  
     updateMessage3 = "MeasureC  "  +  value3 ; 

     String value4 = doc["MeasureD"];  
     updateMessage4 = "MeasureD " +  value4 ;
 
     String value5 = doc["robot"];  
     updateMessage5 =   value5 ;  
     dataUpdated = true;

   }
 }
};


void setup() {
  
 Serial.begin(115200);

 //Initialize screen 
 tft.begin();
 tft.fillScreen(0x0000);
 tft.setRotation(0);  

 // Initialize BLE
 // We use MYWEARABLESNAME to identify the peripheral in our client code.
 BLEDevice::init("MYWEARABLESNAME");
 pServer = BLEDevice::createServer();
 pServer->setCallbacks(new MyServerCallbacks());

 // Create BLE Service
 BLEService *pService = pServer->createService(SERVICE_UUID);

 // Create TX Characteristic (for sending data)
 pTxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_TX,
                       BLECharacteristic::PROPERTY_NOTIFY
                     ); 
 pTxCharacteristic->addDescriptor(new BLE2902());
 
 // Create RX Characteristic (for receiving/ reading data)
 BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE|
                                           BLECharacteristic::PROPERTY_READ
                                         );
 pRxCharacteristic->setCallbacks(new MyCallbacks());

 // Start the service
 pService->start();

 // Start advertising
 pServer->getAdvertising()->start();
 Serial.println("Waiting for a client connection...");
}

void loop() {

  // Notify connected client with incrementing value
  if (dataUpdated) {
      tft.fillScreen(0x0000);
      tft.setTextColor(GC9A01A_GREEN);

      tft.setCursor(60,20);
      tft.setTextSize(1);
      tft.println(updateMessage5); 

      tft.setCursor(40, 40);
      tft.setTextSize(2);
      tft.println(updateMessage1);
 
      tft.setCursor(40,80);
      tft.setTextSize(2);
      tft.println(updateMessage2);

      tft.setCursor(40,120);
      tft.setTextSize(2);
      tft.println(updateMessage3);

      tft.setCursor(40,160);
      tft.setTextSize(2);
      tft.println(updateMessage4); 
  
      dataUpdated = false;
  }

  delay(1000);
 
}
  