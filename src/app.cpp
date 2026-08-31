#include "app.h"

#include <Arduino.h>
#include <M5Unified.h>

#include "config.h"
#include "chain_port.h"
#include "diagnostics.h"
#include "dualkey_hardware.h"
#include "key_settings.h"
#include "network_manager.h"
#include "osc_manager.h"
#include "system_settings.h"

namespace {

unsigned long lastHeartbeatMs = 0;
bool bootDiagnosticsPrinted = false;

}  // namespace

void appSetup() {
  // Match the stable M5ChainOSC startup sequence: initialise M5 board support,
  // bring up networking, and only then initialise local/Chain hardware.
  auto m5Config = M5.config();
  m5Config.serial_baudrate = SERIAL_BAUD;
  M5.begin(m5Config);
  Serial.begin(SERIAL_BAUD);
  delay(200);

  systemSettingsSetup();
  oscSetup();
  networkSetup();
  dualKeyHardwareSetup();
  chainPortSetup();
}

void appLoop() {
  const unsigned long now = millis();

  M5.update();
  networkUpdate();
  dualKeyHardwareUpdate();
  chainPortUpdate();

  if (!bootDiagnosticsPrinted && now >= BOOT_DIAGNOSTICS_DELAY_MS) {
    bootDiagnosticsPrinted = true;
    printBootDiagnostics();
    keySettingsPrintState();
    printHeartbeat();
    lastHeartbeatMs = now;
  } else if (bootDiagnosticsPrinted &&
             now - lastHeartbeatMs >= HEARTBEAT_INTERVAL_MS) {
    lastHeartbeatMs = now;
    printHeartbeat();
  }
  delay(1);
}
