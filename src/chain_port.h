#pragma once

#include <Arduino.h>

void chainPortSetup();
void chainPortUpdate();
bool chainPortIdentifyDevice(const String& identity);
size_t chainPortConnectedDeviceCount();
bool chainPortConnectedDeviceAt(size_t index, String& identity,
                                uint8_t& deviceType);
