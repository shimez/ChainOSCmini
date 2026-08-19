#pragma once

#include <Arduino.h>

enum class NetworkLedState {
  CONNECTING,
  CONNECTED,
  AP_MODE,
};

void dualKeyHardwareSetup();
void dualKeyHardwareUpdate();
void dualKeySetNetworkLedState(NetworkLedState state);
bool dualKeyIdentifyDevice(const String& identity);
