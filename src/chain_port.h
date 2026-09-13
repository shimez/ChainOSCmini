#pragma once

#include <Arduino.h>

void chainPortSetup();
void chainPortUpdate();
bool chainPortIdentifyDevice(const String& identity);
void chainPortResetAngleRuntimeState(const String& identity);
size_t chainPortConnectedDeviceCount();
bool chainPortConnectedDeviceAt(size_t index, String& identity,
                                uint8_t& deviceType);
bool chainPortConnectedDeviceAt(size_t index, String& identity,
                                uint8_t& deviceType, uint8_t& portNumber,
                                size_t& portIndex);
