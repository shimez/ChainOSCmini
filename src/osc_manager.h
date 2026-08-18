#pragma once

#include <Arduino.h>

void oscSetup();
const String& oscTargetHost();
uint16_t oscTargetPort();
bool oscSaveTarget(const String& host, uint16_t port);
void oscSendDualKey(uint8_t keyNumber, bool pressed);
void oscSendChainKey(const uint8_t* uid, size_t uidLength, bool pressed);
