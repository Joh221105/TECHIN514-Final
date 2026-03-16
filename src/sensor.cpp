#include <Arduino.h>
#include "DFRobot_DF2301Q.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ── UART Pins ──────────────────────────────────────────────────────────────
HardwareSerial sensorSerial(1);
DFRobot_DF2301Q_UART asr(&sensorSerial, D10, D9);

// ── BLE UUIDs ──────────────────────────────────────────────────────────────
#define SERVICE_UUID        "1a8ac0fb-de23-4540-9511-a4fa87aaab90"
#define CHARACTERISTIC_UUID "1a8ac0fb-de23-4540-9511-a4fa87aaab91"
#define DEVICE_NAME         "SensorESP32"

// ── Globals ────────────────────────────────────────────────────────────────
BLEServer*         pServer         = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected    = false;
bool oldDeviceConnected = false;

// ── Sliding Window Histogram ───────────────────────────────────────────────
#define HISTORY_SIZE 10
#define HISTORY_WINDOW_MS 5000
#define COOLDOWN_MS 1500

struct CMDEvent { uint8_t id; unsigned long t; };
CMDEvent history[HISTORY_SIZE];
int historyIdx = 0;

uint8_t lastSentID = 0;
unsigned long lastSentTime = 0;

void recordCMD(uint8_t id) {
  history[historyIdx] = { id, millis() };
  historyIdx = (historyIdx + 1) % HISTORY_SIZE;
}

uint8_t countRecent(uint8_t id) {
  unsigned long now = millis();
  uint8_t count = 0;
  for (int i = 0; i < HISTORY_SIZE; i++) {
    if (history[i].id == id && (now - history[i].t) < HISTORY_WINDOW_MS)
      count++;
  }
  return count;
}

uint8_t filteredCMDID(uint8_t id) {
  if (id == 0) return 0;
  unsigned long now = millis();
  if (id == lastSentID && (now - lastSentTime) < COOLDOWN_MS) return 0;
  lastSentID = id;
  lastSentTime = now;
  recordCMD(id);
  return id;
}

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
  delay(500);
  Serial.println("Starting...");

  memset(history, 0, sizeof(history));

  if (!asr.begin()) {
    Serial.println("asr.begin() FAILED - check wiring!");
  } else {
    Serial.println("asr.begin() SUCCESS");
    delay(500);
    asr.settingCMD(DF2301Q_UART_MSG_CMD_SET_MUTE, 1);
    delay(100);
    asr.settingCMD(DF2301Q_UART_MSG_CMD_SET_MUTE, 0);
    delay(100);
    asr.settingCMD(DF2301Q_UART_MSG_CMD_SET_VOLUME, 1);
    delay(100);
    asr.settingCMD(DF2301Q_UART_MSG_CMD_SET_WAKE_TIME, 30);
  }

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

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  BLEDevice::startAdvertising();
  Serial.println("[BLE] Advertising as 'SensorESP32'");
}

// ── Loop ───────────────────────────────────────────════════════════════════
void loop() {
  uint8_t id = filteredCMDID(asr.getCMDID());
  while (asr.getCMDID() != 0) {}  // drain buffer of duplicates

  if (id != 0) {
    Serial.print("Command ID: ");
    Serial.println(id);

    pCharacteristic->setValue(&id, 1);
    if (deviceConnected) {
      pCharacteristic->notify();
      Serial.println("[BLE] Notification sent");
    }
  }

  // ── BLE reconnect ─────────────────────────────────────────────────────
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