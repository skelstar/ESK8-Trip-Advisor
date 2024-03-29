#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

bool clientConnected = false;

class MyServerCallbacks: public BLECharacteristicCallbacks {
	// receive

  #define CLEAR_TRIP_ODO_COMMAND  99

  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0 && value[0] == CLEAR_TRIP_ODO_COMMAND) {
      clearTripMeterAndOdometer();
    }
  }

	void onConnect(BLEServer* pServer) {
		Serial.printf("device connected\n");
    clientConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
		Serial.printf("device disconnected\n");
    clientConnected = false;
  }
};

void setupBLE() {

    BLEDevice::init("Task Advisor - Server");
    BLEServer *pServer = BLEDevice::createServer();

    uint8_t mac5[6];
    esp_read_mac(mac5, ESP_MAC_BT);
    Serial.printf("[Bluetooth] Mac Address = %02X:%02X:%02X:%02X:%02X:%02X\r\n", mac5[0], mac5[1], mac5[2], mac5[3], mac5[4], mac5[5]);

    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
      BLECharacteristic::PROPERTY_WRITE |
      BLECharacteristic::PROPERTY_NOTIFY
    );
	  pCharacteristic->addDescriptor(new BLE2902());

    pCharacteristic->setCallbacks(new MyServerCallbacks());
    pCharacteristic->setValue("Hello World says Neil");
    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); 
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
}

void sendDataToClient() {

	uint8_t bs[sizeof(vescdata)];
	memcpy(bs, &vescdata, sizeof(vescdata));

	pCharacteristic->setValue(bs, sizeof(bs));
	pCharacteristic->notify();
}
