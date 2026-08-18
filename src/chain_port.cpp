#include "chain_port.h"

#include <Arduino.h>
#include <M5Chain.h>
#include <string.h>

#include "config.h"
#include "osc_manager.h"

namespace {

constexpr size_t UID_SIZE = 12;

struct DeviceSnapshot {
  uint16_t id;
  chain_device_type_t type;
  uint8_t uid[UID_SIZE];
  bool uidValid;
  uint8_t lastButtonStatus;
  bool buttonInitialized;
  bool keyReadErrorReported;
};

struct ChainPortContext {
  const char* name;
  HardwareSerial* serial;
  uint8_t rxPin;
  uint8_t txPin;
  Chain bus;
  DeviceSnapshot devices[CHAIN_MAX_DEVICES];
  uint16_t deviceCount;
  bool connected;
  bool firstScan;
  unsigned long lastScanMs;
  unsigned long lastKeyPollMs;

  ChainPortContext(const char* portName, HardwareSerial* hardwareSerial,
                   uint8_t rx, uint8_t tx)
      : name(portName),
        serial(hardwareSerial),
        rxPin(rx),
        txPin(tx),
        devices{},
        deviceCount(0),
        connected(false),
        firstScan(true),
        lastScanMs(0),
        lastKeyPollMs(0) {}
};

ChainPortContext portG5G6("G5_G6", &Serial2, CHAIN_G5_G6_RX_PIN,
                          CHAIN_G5_G6_TX_PIN);
ChainPortContext portG47G48("G47_G48", &Serial1, CHAIN_G47_G48_RX_PIN,
                            CHAIN_G47_G48_TX_PIN);

uint8_t colorBlue[] = {0, 0, 255};
uint8_t colorOrange[] = {255, 64, 0};

const char* chainStatusName(chain_status_t status) {
  switch (status) {
    case CHAIN_OK: return "OK";
    case CHAIN_PARAMETER_ERROR: return "PARAMETER_ERROR";
    case CHAIN_RETURN_PACKET_ERROR: return "RETURN_PACKET_ERROR";
    case CHAIN_BUSY: return "BUSY";
    case CHAIN_TIMEOUT: return "TIMEOUT";
    default: return "UNKNOWN";
  }
}

const char* deviceTypeName(chain_device_type_t type) {
  switch (type) {
    case CHAIN_ENCODER_TYPE_CODE: return "Encoder";
    case CHAIN_ANGLE_TYPE_CODE: return "Angle";
    case CHAIN_KEY_TYPE_CODE: return "Key";
    case CHAIN_JOYSTICK_TYPE_CODE: return "Joystick";
    case CHAIN_TOF_TYPE_CODE: return "ToF";
    case UNIT_CHAIN_BUS_TYPE_CODE: return "UnitChainBus";
    case CHAIN_SWITCH_TYPE_CODE: return "Switch";
    case CHAIN_PIR_TYPE_CODE: return "PIR";
    case CHAIN_MIC_TYPE_CODE: return "Microphone";
    case CHAIN_BUZZER_TYPE_CODE: return "Buzzer";
    case UNIT_8SERVOS2_CHAIN_TYPE_CODE: return "8Servos2";
    case CHAIN_MONO_TYPE_CODE: return "Mono";
    case CHAIN_RGB_TYPE_CODE: return "RGB";
    case CHAIN_UNKNOWN_TYPE_CODE:
    default: return "Unknown";
  }
}

void printUid(const DeviceSnapshot& device) {
  if (!device.uidValid) {
    Serial.print("unavailable");
    return;
  }
  for (size_t index = 0; index < UID_SIZE; ++index) {
    Serial.printf("%02X", device.uid[index]);
  }
}

bool sameDevice(const DeviceSnapshot& left, const DeviceSnapshot& right) {
  if (left.id != right.id || left.type != right.type ||
      left.uidValid != right.uidValid) {
    return false;
  }
  return !left.uidValid || memcmp(left.uid, right.uid, UID_SIZE) == 0;
}

bool snapshotChanged(const ChainPortContext& port,
                     const DeviceSnapshot* devices, uint16_t count) {
  if (!port.connected || count != port.deviceCount) {
    return true;
  }
  for (uint16_t index = 0; index < count; ++index) {
    if (!sameDevice(devices[index], port.devices[index])) {
      return true;
    }
  }
  return false;
}

void saveSnapshot(ChainPortContext& port, const DeviceSnapshot* devices,
                  uint16_t count) {
  port.deviceCount = count;
  for (uint16_t index = 0; index < count; ++index) {
    port.devices[index] = devices[index];
  }
}

void printSnapshot(const ChainPortContext& port,
                   const DeviceSnapshot* devices, uint16_t count) {
  Serial.printf("[ChainOSCmini][CHAIN][%s] devices=%u\n", port.name, count);
  for (uint16_t index = 0; index < count; ++index) {
    Serial.printf(
        "[ChainOSCmini][CHAIN][%s] index=%u id=%u type=%u(%s) uid=",
        port.name, index, devices[index].id,
        static_cast<unsigned int>(devices[index].type),
        deviceTypeName(devices[index].type));
    printUid(devices[index]);
    Serial.println();
  }
}

void drainKeyReports(ChainPortContext& port, uint16_t id) {
  chain_button_press_type_t ignoredType;
  while (port.bus.getKeyButtonPressStatus(id, &ignoredType)) {
    // Raw pressed/released state is used. Active reports are drained because
    // M5Chain stores each one in a dynamically allocated linked-list node.
  }
}

void drainAllKeyReports(ChainPortContext& port) {
  for (uint16_t id = 1; id <= CHAIN_MAX_DEVICES; ++id) {
    drainKeyReports(port, id);
  }
}

bool setKeyLed(ChainPortContext& port, DeviceSnapshot& device,
               uint8_t* color, const char* colorName) {
  uint8_t operationStatus = 0;
  const chain_status_t status = port.bus.setRGBValue(
      device.id, 0, 1, color, 3, &operationStatus, 100);
  if (status == CHAIN_OK && operationStatus != 0) {
    return true;
  }
  Serial.printf(
      "[ChainOSCmini][CHAIN_KEY][%s] led_error id=%u color=%s "
      "status=%d(%s) operation=%u\n",
      port.name, device.id, colorName, static_cast<int>(status),
      chainStatusName(status), operationStatus);
  return false;
}

void initializeKey(ChainPortContext& port, DeviceSnapshot& device) {
  uint8_t operationStatus = 0;
  const chain_status_t brightnessStatus = port.bus.setRGBLight(
      device.id, CHAIN_KEY_LED_BRIGHTNESS, &operationStatus,
      CHAIN_SAVE_FLASH_DISABLE, 100);
  if (brightnessStatus != CHAIN_OK || operationStatus == 0) {
    Serial.printf(
        "[ChainOSCmini][CHAIN_KEY][%s] brightness_error id=%u "
        "status=%d(%s) operation=%u\n",
        port.name, device.id, static_cast<int>(brightnessStatus),
        chainStatusName(brightnessStatus), operationStatus);
  }

  uint8_t rawStatus = 0;
  const chain_status_t status =
      port.bus.getKeyButtonStatus(device.id, &rawStatus, 100);
  drainKeyReports(port, device.id);
  if (status != CHAIN_OK) {
    Serial.printf(
        "[ChainOSCmini][CHAIN_KEY][%s] init_error id=%u status=%d(%s)\n",
        port.name, device.id, static_cast<int>(status),
        chainStatusName(status));
    device.buttonInitialized = false;
    return;
  }

  device.lastButtonStatus = rawStatus != 0 ? 1 : 0;
  device.buttonInitialized = true;
  device.keyReadErrorReported = false;
  setKeyLed(port, device,
            device.lastButtonStatus != 0 ? colorOrange : colorBlue,
            device.lastButtonStatus != 0 ? "ORANGE" : "BLUE");

  Serial.printf("[ChainOSCmini][CHAIN_KEY][%s] ready id=%u uid=", port.name,
                device.id);
  printUid(device);
  Serial.printf(" initial=%s led=%s\n",
                device.lastButtonStatus != 0 ? "PRESSED" : "RELEASED",
                device.lastButtonStatus != 0 ? "ORANGE" : "BLUE");
}

void initializeKeys(ChainPortContext& port) {
  for (uint16_t index = 0; index < port.deviceCount; ++index) {
    if (port.devices[index].type == CHAIN_KEY_TYPE_CODE) {
      initializeKey(port, port.devices[index]);
    }
  }
}

void pollKeys(ChainPortContext& port) {
  if (!port.connected) {
    return;
  }

  for (uint16_t index = 0; index < port.deviceCount; ++index) {
    DeviceSnapshot& device = port.devices[index];
    if (device.type != CHAIN_KEY_TYPE_CODE) {
      continue;
    }

    uint8_t rawStatus = 0;
    const chain_status_t status =
        port.bus.getKeyButtonStatus(device.id, &rawStatus, 50);
    drainKeyReports(port, device.id);
    if (status != CHAIN_OK) {
      if (!device.keyReadErrorReported) {
        Serial.printf(
            "[ChainOSCmini][CHAIN_KEY][%s] read_error id=%u "
            "status=%d(%s)\n",
            port.name, device.id, static_cast<int>(status),
            chainStatusName(status));
        device.keyReadErrorReported = true;
      }
      continue;
    }

    device.keyReadErrorReported = false;
    const uint8_t buttonStatus = rawStatus != 0 ? 1 : 0;
    if (!device.buttonInitialized) {
      device.lastButtonStatus = buttonStatus;
      device.buttonInitialized = true;
      setKeyLed(port, device, buttonStatus != 0 ? colorOrange : colorBlue,
                buttonStatus != 0 ? "ORANGE" : "BLUE");
      continue;
    }
    if (buttonStatus == device.lastButtonStatus) {
      continue;
    }

    device.lastButtonStatus = buttonStatus;
    const bool pressed = buttonStatus != 0;
    setKeyLed(port, device, pressed ? colorOrange : colorBlue,
              pressed ? "ORANGE" : "BLUE");
    Serial.printf("[ChainOSCmini][CHAIN_KEY][%s] id=%u uid=", port.name,
                  device.id);
    printUid(device);
    Serial.printf(" state=%s led=%s\n", pressed ? "PRESSED" : "RELEASED",
                  pressed ? "ORANGE" : "BLUE");
    if (device.uidValid) {
      oscSendChainKey(device.uid, UID_SIZE, pressed);
    }
  }
}

void scanChainPort(ChainPortContext& port) {
  const bool connected = port.bus.isDeviceConnected(1, 20);
  if (!connected) {
    if (port.connected || port.firstScan) {
      Serial.printf(
          "[ChainOSCmini][CHAIN][%s] state=DISCONNECTED devices=0\n",
          port.name);
    }
    port.connected = false;
    port.deviceCount = 0;
    drainAllKeyReports(port);
    port.firstScan = false;
    return;
  }

  uint16_t reportedCount = 0;
  const chain_status_t countStatus =
      port.bus.getDeviceNum(&reportedCount, 150);
  if (countStatus != CHAIN_OK) {
    Serial.printf(
        "[ChainOSCmini][CHAIN][%s] scan_error=get_count status=%d(%s) "
        "previous_state_retained=true\n",
        port.name, static_cast<int>(countStatus),
        chainStatusName(countStatus));
    return;
  }

  if (reportedCount > CHAIN_MAX_DEVICES) {
    Serial.printf(
        "[ChainOSCmini][CHAIN][%s] scan_error=too_many reported=%u max=%u\n",
        port.name, reportedCount, CHAIN_MAX_DEVICES);
    return;
  }

  device_info_t deviceInfo[CHAIN_MAX_DEVICES] = {};
  device_list_t list = {reportedCount, deviceInfo};
  if (reportedCount > 0 && !port.bus.getDeviceList(&list, 150)) {
    Serial.printf("[ChainOSCmini][CHAIN][%s] scan_error=get_list\n",
                  port.name);
    return;
  }

  DeviceSnapshot currentDevices[CHAIN_MAX_DEVICES] = {};
  bool uidError = false;
  for (uint16_t index = 0; index < reportedCount; ++index) {
    currentDevices[index].id = deviceInfo[index].id;
    currentDevices[index].type = deviceInfo[index].device_type;

    uint8_t operationStatus = 0;
    const chain_status_t uidStatus = port.bus.getUID(
        currentDevices[index].id, UID_TYPE_12_BYTE,
        currentDevices[index].uid, UID_SIZE, &operationStatus, 150);
    currentDevices[index].uidValid =
        uidStatus == CHAIN_OK && operationStatus != 0;
    if (!currentDevices[index].uidValid) {
      Serial.printf(
          "[ChainOSCmini][CHAIN][%s] scan_error=get_uid id=%u "
          "status=%d(%s) operation=%u previous_state_retained=true\n",
          port.name, currentDevices[index].id, static_cast<int>(uidStatus),
          chainStatusName(uidStatus), operationStatus);
      uidError = true;
    }
  }
  if (uidError) {
    return;
  }

  const bool changed = snapshotChanged(port, currentDevices, reportedCount);
  if (changed) {
    Serial.printf("[ChainOSCmini][CHAIN][%s] state=%s\n", port.name,
                  port.connected ? "CHANGED" : "CONNECTED");
    printSnapshot(port, currentDevices, reportedCount);
    drainAllKeyReports(port);
    saveSnapshot(port, currentDevices, reportedCount);
    port.connected = true;
    initializeKeys(port);
  }
  port.connected = true;
  port.firstScan = false;
}

void setupPort(ChainPortContext& port) {
  port.bus.begin(port.serial, CHAIN_BAUD, port.rxPin, port.txPin);
  Serial.printf(
      "[ChainOSCmini][CHAIN][%s] RX=%u TX=%u baud=%lu enabled=true\n",
      port.name, port.rxPin, port.txPin,
      static_cast<unsigned long>(CHAIN_BAUD));
  port.lastScanMs = millis();
  port.lastKeyPollMs = millis();
}

void updatePort(ChainPortContext& port, unsigned long now) {
  if (port.firstScan && now < BOOT_DIAGNOSTICS_DELAY_MS) {
    return;
  }
  if (now - port.lastScanMs >= CHAIN_SCAN_INTERVAL_MS) {
    port.lastScanMs = now;
    scanChainPort(port);
  }
  if (now - port.lastKeyPollMs >= CHAIN_KEY_POLL_INTERVAL_MS) {
    port.lastKeyPollMs = now;
    pollKeys(port);
  }
}

}  // namespace

void chainPortSetup() {
  setupPort(portG5G6);
  setupPort(portG47G48);
  Serial.println("[ChainOSCmini][CHAIN] dual_port=true");
}

void chainPortUpdate() {
  const unsigned long now = millis();
  updatePort(portG5G6, now);
  updatePort(portG47G48, now);
}
