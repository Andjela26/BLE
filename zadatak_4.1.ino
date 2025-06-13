#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <BLE2902.h>
#include "DHT11.h"

#define BLE_DEVICE_NAME "ESP32 BLE Server 123"
#define SERVICE_UUID "030ff29f-0348-4e87-9550-ccf1cfdf615d"
#define CHARACTERISTIC_1_UUID "db79d281-1563-4454-9daf-5351ef61d3d7"
#define CHARACTERISTIC_2_UUID "e8942db4-98ac-4b9c-8bc9-3c6b8843ed42"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic1 = NULL; //karekteristika vezana za temperaturu
BLECharacteristic* pCharacteristic2 = NULL; //karakteristika vezana za periodu slanja notifikacije

uint16_t temperature;  //vrednost renutne temperature

const int THM_PIN = 2; //pin tastera

const int RED_LED_PIN = 21; //pin LED diode
bool RedledState = LOW;

const int GREEN_LED_PIN = 20; //pin LED diode
bool GreenledState = LOW;

uint16_t notificationPeriod = 5; //period slanja notifikacija
unsigned long currentTime = 0; //tekuce vreme u ms
unsigned long lastNotifyTime = 0;//tren. posl. LED blinka

int deviceConnected = LOW; //stanje BLE konekcije
int lastDeviceConnected = LOW; //prethodno stanje BLE konekcije
 
DHT11 dht11(THM_PIN);
 
class bleServerCallbacks: public BLEServerCallbacks {
 void onConnect(BLEServer* pServer) {
 deviceConnected = HIGH;
 };
 void onDisconnect(BLEServer* pServer) {
 deviceConnected = LOW;
 }
};
 
//Callback funkcije karakteristike za periodu slanja notifikacija
class char1Callbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
  std::string value = pCharacteristic->getValue();
  notificationPeriod = (value[1] << 8) + value[0];
  }
};
//Callback funkcije karakteristike za citanje temperature
class char2Callbacks: public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic){
  RedledState = HIGH;
  digitalWrite(RED_LED_PIN,RedledState);
  delay(50);
  RedledState = LOW;
  digitalWrite(RED_LED_PIN,RedledState);
  }
};
 
//Inicijalizacija BLE i GATT
void init_service(){
  //Inicijalizacija i startovanje BLE servera
  BLEDevice::init(BLE_DEVICE_NAME);
  pServer = BLEDevice::createServer();
  //Kreiranje GATT servisa
  pServer->setCallbacks(new bleServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  //Kreiranje karakteristike za periodu slanja notifikacija
  pCharacteristic1 = pService->createCharacteristic(
                                                  CHARACTERISTIC_1_UUID,
                                                  BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic1->setValue((uint8_t*)&notificationPeriod, 2);
  pCharacteristic1->setCallbacks(new char1Callbacks());
 
  //Kreiranje karakteristike za temperaturu
  pCharacteristic2 = pService->createCharacteristic(
                                                  CHARACTERISTIC_2_UUID,
                                                  BLECharacteristic::PROPERTY_READ |
                                                  BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic2->addDescriptor(new BLE2902());
  pCharacteristic2->setValue((uint8_t*)&temperature, 2);
  pCharacteristic2->setCallbacks(new char2Callbacks());
 
  //Startovanje GATT servera
  pService->start();
  Serial.println("GATT service started");
}
 
//Startovanje oglasavanja
void start_advertising(){
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.println("Advertising started");
}
 
void setup() {
  
  pinMode(THM_PIN, INPUT_PULLDOWN);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  Serial.begin(115200);
  init_service();
  start_advertising();
}
 
void loop() {
  currentTime = millis();
  //Pracenje statusa BLE konekcije
  if(deviceConnected != lastDeviceConnected){
    if(deviceConnected == HIGH){
      GreenledState = HIGH;
      digitalWrite(GREEN_LED_PIN, GreenledState);
  
    }
    else{
    //BLE konekcija je raskinuta
    GreenledState = LOW;
    digitalWrite(GREEN_LED_PIN, GreenledState);
    start_advertising();
    }
  lastDeviceConnected = deviceConnected;
  }
  if((notificationPeriod>= 1) && (notificationPeriod <= 255)){
    if((currentTime - lastNotifyTime) >= notificationPeriod*1000 ){
      temperature = dht11.readTemperature();
      lastNotifyTime = currentTime;
      pCharacteristic2->setValue((uint8_t*)&temperature, 2);
      if(deviceConnected == HIGH){
        pCharacteristic2->notify();
        RedledState = HIGH;
        digitalWrite(RED_LED_PIN,RedledState);
        delay(50);
        RedledState = LOW;
        digitalWrite(RED_LED_PIN,RedledState);
      }
    }
  }
  else {
    Serial.println("Unet period slanja notifikacija nije u odgovarajućem opsegu");
  }
}
