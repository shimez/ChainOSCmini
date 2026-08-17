#include "app.h"

#include <Arduino.h>

#include "config.h"
#include "diagnostics.h"

namespace {

unsigned long lastHeartbeatMs = 0;

}  // namespace

void appSetup() {
  Serial.begin(SERIAL_BAUD);

  const unsigned long waitStartedMs = millis();
  while (!Serial && millis() - waitStartedMs < SERIAL_WAIT_MS) {
    delay(10);
  }

  printBootDiagnostics();
  lastHeartbeatMs = millis();
}

void appLoop() {
  const unsigned long now = millis();
  if (now - lastHeartbeatMs >= HEARTBEAT_INTERVAL_MS) {
    lastHeartbeatMs = now;
    printHeartbeat();
  }
  delay(10);
}
