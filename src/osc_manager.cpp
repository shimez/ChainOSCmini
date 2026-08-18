#include "osc_manager.h"

#include <ArduinoOSCWiFi.h>
#include <Preferences.h>
#include <WiFi.h>
#include <math.h>

#include "config.h"
#include "key_settings.h"

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

void sendMessage(const KeySetting& setting, const KeyOscMessage& message) {
  if (message.valueType == TYPE_FLOAT) {
    OscWiFi.send(targetHost.c_str(), targetPort, message.address.c_str(),
                 message.valueStr.toFloat());
  } else if (message.valueType == TYPE_INT) {
    OscWiFi.send(targetHost.c_str(), targetPort, message.address.c_str(),
                 message.valueStr.toInt());
  } else {
    OscWiFi.send(targetHost.c_str(), targetPort, message.address.c_str(),
                 message.valueStr.c_str());
  }
  Serial.printf("[ChainOSCmini][OSC] source=%s address=%s value=%s target=%s:%u\n",
                setting.identity.c_str(), message.address.c_str(),
                message.valueStr.c_str(), targetHost.c_str(), targetPort);
}

void sendKeyValue(KeySetting& setting, bool pressed) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("[ChainOSCmini][OSC] source=%s skipped=wifi_disconnected\n",
                  setting.identity.c_str());
    return;
  }

  if (setting.mode == MODE_SEQUENCE) {
    if (!pressed) return;
    KeySequenceConfig& sequence = setting.sequence;
    const float value = sequence.current;
    const String valueText = sequence.valueType == TYPE_INT
                                 ? String(static_cast<int>(lroundf(value)))
                                 : String(value, 3);
    if (sequence.valueType == TYPE_FLOAT)
      OscWiFi.send(targetHost.c_str(), targetPort, sequence.address.c_str(), value);
    else if (sequence.valueType == TYPE_INT)
      OscWiFi.send(targetHost.c_str(), targetPort, sequence.address.c_str(),
                   static_cast<int>(lroundf(value)));
    else
      OscWiFi.send(targetHost.c_str(), targetPort, sequence.address.c_str(),
                   valueText.c_str());
    float next = value + sequence.step;
    if ((sequence.step >= 0 && next > sequence.end + 1e-6f) ||
        (sequence.step < 0 && next < sequence.end - 1e-6f))
      next = sequence.start;
    sequence.current = next;
    Serial.printf("[ChainOSCmini][OSC] source=%s mode=sequence address=%s value=%s target=%s:%u\n",
                  setting.identity.c_str(), sequence.address.c_str(),
                  valueText.c_str(), targetHost.c_str(), targetPort);
    return;
  }

  KeyOscMessage* messages = pressed ? setting.pressMessages : setting.releaseMessages;
  const uint8_t count = pressed ? setting.pressMessageCount : setting.releaseMessageCount;
  for (uint8_t index = 0; index < count; ++index) sendMessage(setting, messages[index]);
}

}  // namespace

void oscSetup() {
  keySettingsSetup();
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
  const String identity = String("dualkey:") + keyNumber;
  KeySetting* setting = keySettingsEnsure(
      identity, String("DualKey KEY") + keyNumber,
      String("/chainoscmini/dualkey/key") + keyNumber);
  if (setting != nullptr) sendKeyValue(*setting, pressed);
}

void oscBeginChainPortUpdate(uint8_t portMask) {
  keySettingsBeginPortUpdate(portMask);
}

void oscRegisterChainKey(const uint8_t* uidBytes, size_t uidLength,
                         uint8_t portMask) {
  const String uid = uidText(uidBytes, uidLength);
  const String identity = String("chain:") + uid;
  keySettingsEnsure(identity, String("Chain Key ") + uid,
                    String("/chainoscmini/chain/key/") + uid);
  keySettingsMarkConnected(identity, portMask);
}

void oscSendChainKey(const uint8_t* uidBytes, size_t uidLength, bool pressed) {
  const String uid = uidText(uidBytes, uidLength);
  KeySetting* setting = keySettingsEnsure(
      String("chain:") + uid, String("Chain Key ") + uid,
      String("/chainoscmini/chain/key/") + uid);
  if (setting != nullptr) sendKeyValue(*setting, pressed);
}
