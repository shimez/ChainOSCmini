#pragma once

enum class NetworkLedState {
  CONNECTING,
  CONNECTED,
  AP_MODE,
};

void dualKeyHardwareSetup();
void dualKeyHardwareUpdate();
void dualKeySetNetworkLedState(NetworkLedState state);
