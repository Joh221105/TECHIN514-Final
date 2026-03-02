#include <Arduino.h>
#include "DFRobot_DF2301Q.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ── UART Pins ──────────────────────────────────────────────────────────────
HardwareSerial sensorSerial(1);
DFRobot_DF2301Q_UART asr(&sensorSerial, D9, D10);

// ── BLE UUIDs ──────────────────────────────────────────────────────────────
#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID "12345678-1234-5678-1234-56789abcdef1"
#define DEVICE_NAME         "SensorESP32"

// ── Globals ────────────────────────────────────────────────────────────────
BLEServer*         pServer         = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected    = false;
bool oldDeviceConnected = false;

// ── BLE Server Callbacks ───────────────────────────────────────────────────
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    deviceConnected = true;
    Serial.println("[BLE] Display connected");
  }
  void onDisconnect(BLEServer* pServer) override {
    deviceConnected = false;
    Serial.println("[BLE] Display disconnected");
  }
};

// ── Setup ──────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(500);  // brief settle; do NOT block on while(!Serial) — kills BLE without monitor
  Serial.println("Starting...");

  // — SEN0539 via UART —
  if (!asr.begin()) {
    Serial.println("asr.begin() FAILED - check wiring!");
  } else {
    Serial.println("asr.begin() SUCCESS");
  }

  // — BLE —
  BLEDevice::init(DEVICE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());

  uint8_t initVal = 0;
  pCharacteristic->setValue(&initVal, 1);
  pService->start();

  // Let the library auto-build the advert packet from addServiceUUID().
  // DO NOT call setAdvertisementData() — it replaces the entire payload and
  // strips the service UUID, making isAdvertisingService() fail on the scanner.
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);  // scan response carries the device name
  pAdvertising->setMinPreferred(0x06);  // iOS connection hint
  BLEDevice::startAdvertising();
  Serial.println("[BLE] Advertising as 'SensorESP32'");
}

// ── Loop ───────────────────────────────────────────────────────────────────
void loop() {
  uint8_t id = asr.getCMDID();

  if (id != 0) {
    Serial.print("Command ID: ");
    Serial.println(id);

    pCharacteristic->setValue(&id, 1);
    if (deviceConnected) {
      pCharacteristic->notify();
      Serial.println("[BLE] Notification sent");
    }
  }

  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("[BLE] Restarting advertising...");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  delay(100);
}