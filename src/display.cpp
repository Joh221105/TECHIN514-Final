#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>
#include <BLEAdvertisedDevice.h>

#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID "12345678-1234-5678-1234-56789abcdef1"

static bool doConnect = false;
static bool connected = false;
static bool doScan    = true;
static BLERemoteCharacteristic* pRemoteCharacteristic = nullptr;
static BLEAdvertisedDevice*     myDevice              = nullptr;

static void notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
  if (length >= 1) {
    Serial.print("Command received: ");
    Serial.println(pData[0]);
  }
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) override {}
  void onDisconnect(BLEClient* pclient) override {
    connected = false;
    Serial.println("[BLE] Disconnected");
  }
};

bool connectToServer() {
  Serial.print("[BLE] Connecting to ");
  Serial.println(myDevice->getAddress().toString().c_str());

  // Create a fresh client each attempt — avoids stale GATT CIF on retry
  BLEClient* pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());

  pClient->connect(myDevice);  // pass full device so address type is handled correctly
  Serial.println("[BLE] Connected to server");
  pClient->setMTU(517);

  BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
  if (pRemoteService == nullptr) {
    Serial.println("[BLE] Service UUID not found");
    pClient->disconnect();
    return false;
  }

  pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.println("[BLE] Characteristic UUID not found");
    pClient->disconnect();
    return false;
  }

  if (pRemoteCharacteristic->canNotify()) {
    pRemoteCharacteristic->registerForNotify(notifyCallback);
    Serial.println("[BLE] Subscribed. Waiting for commands...");
  }

  connected = true;
  return true;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    Serial.print("[SCAN] Found: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (advertisedDevice.haveServiceUUID() &&
        advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
      BLEDevice::getScan()->stop();
      myDevice   = new BLEAdvertisedDevice(advertisedDevice);
      doConnect  = true;
      doScan     = true;
    }
  }
};

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Starting display device...");

  BLEDevice::init("DisplayESP32");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void loop() {
  if (doConnect) {
    if (connectToServer()) {
      Serial.println("[BLE] Now connected to sensor.");
    } else {
      Serial.println("[BLE] Connection failed.");
    }
    doConnect = false;
  }

  if (!connected && doScan) {
    BLEDevice::getScan()->start(5, false);
  }

  delay(1000);
}
