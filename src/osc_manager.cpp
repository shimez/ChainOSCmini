#include "osc_manager.h"

#include <ArduinoOSCWiFi.h>
#include <Preferences.h>
#include <WiFi.h>

#include "config.h"

namespace {

String targetHost = "192.168.1.100";
uint16_t targetPort = 9000;

String uidText(const uint8_t* uid, size_t length) {
  String text;
  text.reserve(length * 2);
  for (size_t index = 0; index < length; ++index) {
    char byteText[3];
    snprintf(byteText, sizeof(byteText), "%02X", uid[index]);
    text += byteText;
  }
  return text;
}

void sendKeyValue(const String& source, const String& address, bool pressed) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("[ChainOSCmini][OSC] source=%s skipped=wifi_disconnected\n",
                  source.c_str());
    return;
  }

  OscWiFi.send(targetHost.c_str(), targetPort, address.c_str(),
               pressed ? 1 : 0);
  Serial.printf(
      "[ChainOSCmini][OSC] source=%s address=%s value=%d target=%s:%u\n",
      source.c_str(), address.c_str(), pressed ? 1 : 0, targetHost.c_str(),
      targetPort);
}

}  // namespace

void oscSetup() {
  Preferences preferences;
  if (preferences.begin(WIFI_PREFS_NAMESPACE, true)) {
    targetHost = preferences.getString("osc_host", targetHost);
    const uint32_t storedPort = preferences.getUInt("osc_port", targetPort);
    if (storedPort >= 1 && storedPort <= 65535) {
      targetPort = static_cast<uint16_t>(storedPort);
    }
    preferences.end();
  }
  Serial.printf("[ChainOSCmini][OSC] target=%s:%u\n", targetHost.c_str(),
                targetPort);
}

const String& oscTargetHost() { return targetHost; }

uint16_t oscTargetPort() { return targetPort; }

bool oscSaveTarget(const String& host, uint16_t port) {
  if (host.isEmpty() || host.length() > 253 || port == 0) {
    return false;
  }
  Preferences preferences;
  if (!preferences.begin(WIFI_PREFS_NAMESPACE, false)) {
    return false;
  }
  const size_t hostWritten = preferences.putString("osc_host", host);
  const size_t portWritten = preferences.putUInt("osc_port", port);
  preferences.end();
  if (hostWritten == 0 || portWritten == 0) {
    return false;
  }
  targetHost = host;
  targetPort = port;
  Serial.printf("[ChainOSCmini][OSC] target_saved=%s:%u\n",
                targetHost.c_str(), targetPort);
  return true;
}

void oscSendDualKey(uint8_t keyNumber, bool pressed) {
  const String source = String("DUALKEY_KEY") + keyNumber;
  const String address = String("/chainoscmini/dualkey/key") + keyNumber;
  sendKeyValue(source, address, pressed);
}

void oscSendChainKey(const uint8_t* uidBytes, size_t uidLength, bool pressed) {
  const String uid = uidText(uidBytes, uidLength);
  sendKeyValue(String("CHAIN_KEY_") + uid,
               String("/chainoscmini/chain/key/") + uid, pressed);
}
