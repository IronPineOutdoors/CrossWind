#include "ble_control.h"

#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "diagnostics.h"

static BLECharacteristic* statusCharacteristic = nullptr;
static BLECharacteristic* responseCharacteristic = nullptr;
static BleCommandHandler commandHandler = nullptr;
static bool clientConnected = false;

static String normalizeToken(String token) {
  token.trim();
  token.toUpperCase();
  return token;
}

class CommandCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override {
    String payload = characteristic->getValue();
    payload.trim();
    if (payload.length() == 0) {
      return;
    }

    int separator = payload.indexOf('=');
    if (separator < 0) {
      separator = payload.indexOf(' ');
    }

    String command = payload;
    String value = "";
    if (separator >= 0) {
      command = payload.substring(0, separator);
      value = payload.substring(separator + 1);
    }

    command = normalizeToken(command);
    value.trim();

    if (!commandHandler || !commandHandler(command, value)) {
      sendBleResponse("ERROR", "INVALID_COMMAND_OR_VALUE");
    }
  }
};

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* server) override {
    (void)server;
    clientConnected = true;
  }

  void onDisconnect(BLEServer* server) override {
    (void)server;
    clientConnected = false;
    BLEDevice::startAdvertising();
  }
};

void beginBle(BleCommandHandler handler) {
  commandHandler = handler;

  BLEDevice::init(BLE_DEVICE_NAME);
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService* service = server->createService(BLE_SERVICE_UUID);

  BLECharacteristic* commandCharacteristic = service->createCharacteristic(
    BLE_COMMAND_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  commandCharacteristic->setCallbacks(new CommandCallback());

  statusCharacteristic = service->createCharacteristic(
    BLE_STATUS_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  statusCharacteristic->addDescriptor(new BLE2902());

  responseCharacteristic = service->createCharacteristic(
    BLE_RESPONSE_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  responseCharacteristic->addDescriptor(new BLE2902());
  responseCharacteristic->setValue("status=OK;message=READY");

  service->start();
  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(BLE_SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->start();
}

void updateBleStatus(const ControllerState& state) {
  if (!statusCharacteristic) {
    return;
  }

  String payload = buildStatusPayload(state);
  statusCharacteristic->setValue(payload.c_str());
  if (clientConnected) {
    statusCharacteristic->notify();
  }
}

void sendBleResponse(const String& status, const String& message) {
  String payload = "status=" + status + ";message=" + message;
  if (responseCharacteristic) {
    responseCharacteristic->setValue(payload.c_str());
    if (clientConnected) {
      responseCharacteristic->notify();
    }
  }
  Serial.println(payload);
}
