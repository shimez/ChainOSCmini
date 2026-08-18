#include "chain_port.h"

#include <Arduino.h>
#include <M5Chain.h>
#include <string.h>

#include "config.h"

namespace {

constexpr size_t UID_SIZE = 12;

struct DeviceSnapshot {
  uint16_t id;
  chain_device_type_t type;
  uint8_t uid[UID_SIZE];
  bool uidValid;
};

Chain chainBus;
DeviceSnapshot previousDevices[CHAIN_MAX_DEVICES] = {};
uint16_t previousCount = 0;
bool previousConnected = false;
bool firstScan = true;
unsigned long lastScanMs = 0;

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

bool snapshotChanged(const DeviceSnapshot* devices, uint16_t count) {
  if (!previousConnected || count != previousCount) {
    return true;
  }
  for (uint16_t index = 0; index < count; ++index) {
    if (!sameDevice(devices[index], previousDevices[index])) {
      return true;
    }
  }
  return false;
}

void saveSnapshot(const DeviceSnapshot* devices, uint16_t count) {
  previousCount = count;
  for (uint16_t index = 0; index < count; ++index) {
    previousDevices[index] = devices[index];
  }
}

void printSnapshot(const DeviceSnapshot* devices, uint16_t count) {
  Serial.printf("[ChainOSCmini][CHAIN] devices=%u\n", count);
  for (uint16_t index = 0; index < count; ++index) {
    Serial.printf("[ChainOSCmini][CHAIN] index=%u id=%u type=%u(%s) uid=",
                  index, devices[index].id,
                  static_cast<unsigned int>(devices[index].type),
                  deviceTypeName(devices[index].type));
    printUid(devices[index]);
    Serial.println();
  }
}

void scanChainPort() {
  const bool connected = chainBus.isDeviceConnected(1, 20);
  if (!connected) {
    if (previousConnected || firstScan) {
      Serial.println("[ChainOSCmini][CHAIN] state=DISCONNECTED devices=0");
    }
    previousConnected = false;
    previousCount = 0;
    firstScan = false;
    return;
  }

  uint16_t reportedCount = 0;
  const chain_status_t countStatus =
      chainBus.getDeviceNum(&reportedCount, 150);
  if (countStatus != CHAIN_OK) {
    Serial.printf(
        "[ChainOSCmini][CHAIN] scan_error=get_count status=%d(%s) "
        "previous_state_retained=true\n",
        static_cast<int>(countStatus), chainStatusName(countStatus));
    return;
  }

  if (reportedCount > CHAIN_MAX_DEVICES) {
    Serial.printf(
        "[ChainOSCmini][CHAIN] scan_error=too_many reported=%u max=%u\n",
        reportedCount, CHAIN_MAX_DEVICES);
    return;
  }

  device_info_t deviceInfo[CHAIN_MAX_DEVICES] = {};
  device_list_t list = {reportedCount, deviceInfo};
  if (reportedCount > 0 && !chainBus.getDeviceList(&list, 150)) {
    Serial.println("[ChainOSCmini][CHAIN] scan_error=get_list");
    return;
  }

  DeviceSnapshot currentDevices[CHAIN_MAX_DEVICES] = {};
  bool uidError = false;
  for (uint16_t index = 0; index < reportedCount; ++index) {
    currentDevices[index].id = deviceInfo[index].id;
    currentDevices[index].type = deviceInfo[index].device_type;

    uint8_t operationStatus = 0;
    const chain_status_t uidStatus =
        chainBus.getUID(currentDevices[index].id, UID_TYPE_12_BYTE,
                        currentDevices[index].uid, UID_SIZE,
                        &operationStatus, 150);
    currentDevices[index].uidValid =
        uidStatus == CHAIN_OK && operationStatus != 0;
    if (!currentDevices[index].uidValid) {
      Serial.printf(
          "[ChainOSCmini][CHAIN] scan_error=get_uid id=%u status=%d(%s) "
          "operation=%u previous_state_retained=true\n",
          currentDevices[index].id, static_cast<int>(uidStatus),
          chainStatusName(uidStatus), operationStatus);
      uidError = true;
    }
  }

  // Never replace a valid snapshot with a partial result from a hot-plug
  // transition. The next periodic scan will try the complete enumeration again.
  if (uidError) {
    return;
  }

  if (snapshotChanged(currentDevices, reportedCount)) {
    Serial.printf("[ChainOSCmini][CHAIN] state=%s\n",
                  previousConnected ? "CHANGED" : "CONNECTED");
    printSnapshot(currentDevices, reportedCount);
  }

  saveSnapshot(currentDevices, reportedCount);
  previousConnected = true;
  firstScan = false;
}

}  // namespace

void chainPortSetup() {
  chainBus.begin(&Serial2, CHAIN_BAUD, CHAIN_TEST_RX_PIN,
                 CHAIN_TEST_TX_PIN);
  Serial.printf(
      "[ChainOSCmini][CHAIN] test_port=GPIO5/GPIO6 RX=%u TX=%u baud=%lu\n",
      CHAIN_TEST_RX_PIN, CHAIN_TEST_TX_PIN,
      static_cast<unsigned long>(CHAIN_BAUD));
  Serial.println("[ChainOSCmini][CHAIN] second_port=disabled");
  lastScanMs = millis();
}

void chainPortUpdate() {
  const unsigned long now = millis();
  // Keep the first result visible to serial monitors that attach after reset.
  if (firstScan && now < BOOT_DIAGNOSTICS_DELAY_MS) {
    return;
  }
  if (now - lastScanMs < CHAIN_SCAN_INTERVAL_MS) {
    return;
  }
  lastScanMs = now;
  scanChainPort();
}
