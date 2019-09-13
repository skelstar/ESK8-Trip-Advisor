#include "BLEDevice.h"

/* ---------------------------------------------- */
static BLEAddress *pServerAddress;
static boolean doConnect = false;
static BLERemoteCharacteristic *pRemoteCharacteristic;

class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient *pclient)
  {
    bleConnected();
  }

  void onDisconnect(BLEClient *pclient)
  {
    bleDisconnected();
  }
};

static void notifyCallback(
  BLERemoteCharacteristic *pBLERemoteCharacteristic,
  uint8_t *pData,
  size_t length,
  bool isNotify)
{
  memcpy(&vescdata, pData, sizeof(vescdata));
  bleReceivedNotify();
}

bool bleConnectToServer()
{
  BLEDevice::init("Trip Advisor Client");
  pServerAddress = new BLEAddress("80:7D:3A:C5:6A:36");
  delay(200);
  BLEClient *pClient = BLEDevice::createClient();
  Serial.printf("Client created..\n");
  pClient->setClientCallbacks(new MyClientCallback());
  pClient->connect(*pServerAddress);
  Serial.printf("Connecting..\n");
  delay(500);
  BLERemoteService *pRemoteService = pClient->getService(SERVICE_UUID);
  pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);
  if (pRemoteCharacteristic->canNotify())
  {
    pRemoteCharacteristic->registerForNotify(notifyCallback);
  }
  return true;
}
