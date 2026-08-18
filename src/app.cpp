#include "app.h"

#include <Arduino.h>

#include "config.h"
#include "diagnostics.h"
#include "dualkey_hardware.h"

namespace {

unsigned long lastHeartbeatMs = 0;
bool bootDiagnosticsPrinted = false;

}  // namespace

void appSetup() {
  Serial.begin(SERIAL_BAUD);

  const unsigned long waitStartedMs = millis();
  while (!Serial && millis() - waitStartedMs < SERIAL_WAIT_MS) {
    delay(10);
  }

  dualKeyHardwareSetup();
}

void appLoop() {
  const unsigned long now = millis();

  dualKeyHardwareUpdate();

  if (!bootDiagnosticsPrinted && now >= BOOT_DIAGNOSTICS_DELAY_MS) {
    bootDiagnosticsPrinted = true;
    printBootDiagnostics();
    printHeartbeat();
    lastHeartbeatMs = now;
  } else if (bootDiagnosticsPrinted &&
             now - lastHeartbeatMs >= HEARTBEAT_INTERVAL_MS) {
    lastHeartbeatMs = now;
    printHeartbeat();
  }
  delay(1);
}
