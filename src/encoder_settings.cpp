#include "encoder_settings.h"

#include <Preferences.h>
#include <ctype.h>
#include <math.h>

namespace {

constexpr size_t MAX_ENCODER_SETTINGS = 40;
constexpr char STORAGE_VERSION[] = "E1";
EncoderSetting settings[MAX_ENCODER_SETTINGS];
size_t settingCount = 0;
bool loadingKnown = false;

uint32_t identityHash(const String& identity) {
  uint32_t hash = 2166136261u;
  for (size_t i = 0; i < identity.length(); ++i) {
    hash ^= static_cast<uint8_t>(identity[i]);
    hash *= 16777619u;
  }
  return hash;
}

String deviceNamespace(const String& identity) {
  char name[11];
  snprintf(name, sizeof(name), "e%08X",
           static_cast<unsigned>(identityHash(identity)));
  return String(name);
}

String indexedKey(const char* prefix, uint8_t index) {
  return String(prefix) + String(static_cast<unsigned>(index));
}

bool validAddress(const String& address) {
  if (address.isEmpty() || address.length() > 192 || address[0] != '/')
    return false;
  for (size_t i = 0; i < address.length(); ++i) {
    const char c = address[i];
    if (isspace(static_cast<unsigned char>(c)) || c == '#' || c == '*' ||
        c == ',' || c == '?' || c == '[' || c == ']' || c == '{' || c == '}')
      return false;
  }
  return true;
}

bool sameMessage(const KeyOscMessage& left, const KeyOscMessage& right) {
  return left.address == right.address && left.valueStr == right.valueStr &&
         left.valueType == right.valueType;
}

bool sameSetting(const EncoderSetting& left, const EncoderSetting& right) {
  if (left.identity != right.identity || left.displayName != right.displayName ||
      left.rotationAddress != right.rotationAddress ||
      left.sendIncrement != right.sendIncrement ||
      fabsf(left.absoluteInputMin - right.absoluteInputMin) > 0.00001f ||
      fabsf(left.absoluteInputMax - right.absoluteInputMax) > 0.00001f ||
      fabsf(left.incrementScale - right.incrementScale) > 0.00001f ||
      fabsf(left.outputMin - right.outputMin) > 0.00001f ||
      fabsf(left.outputMax - right.outputMax) > 0.00001f ||
      left.outputType != right.outputType || left.clickMode != right.clickMode ||
      left.pressMessageCount != right.pressMessageCount ||
      left.releaseMessageCount != right.releaseMessageCount ||
      left.clickSequence.address != right.clickSequence.address ||
      left.clickSequence.valueType != right.clickSequence.valueType ||
      fabsf(left.clickSequence.start - right.clickSequence.start) > 0.00001f ||
      fabsf(left.clickSequence.end - right.clickSequence.end) > 0.00001f ||
      fabsf(left.clickSequence.step - right.clickSequence.step) > 0.00001f)
    return false;
  for (uint8_t i = 0; i < left.pressMessageCount; ++i)
    if (!sameMessage(left.pressMessages[i], right.pressMessages[i])) return false;
  for (uint8_t i = 0; i < left.releaseMessageCount; ++i)
    if (!sameMessage(left.releaseMessages[i], right.releaseMessages[i])) return false;
  return true;
}

bool loadSetting(const String& identity, EncoderSetting& setting, bool& found) {
  found = false;
  Preferences preferences;
  const String nameSpace = deviceNamespace(identity);
  if (!preferences.begin(nameSpace.c_str(), true)) return false;
  if (preferences.getString("ver", "") != STORAGE_VERSION) {
    preferences.end();
    return false;
  }
  found = true;
  EncoderSetting candidate = setting;
  if (preferences.getString("id", "") != identity) {
    preferences.end();
    return false;
  }
  candidate.displayName = preferences.getString("name", "");
  candidate.rotationAddress = preferences.getString("rot", "");
  candidate.sendIncrement = preferences.getBool("inc", false);
  candidate.absoluteInputMin = preferences.getFloat("amin", 0);
  candidate.absoluteInputMax = preferences.getFloat("amax", 20);
  candidate.incrementScale = preferences.getFloat("iscale", 0.05f);
  candidate.outputMin = preferences.getFloat("omin", 0);
  candidate.outputMax = preferences.getFloat("omax", 1);
  candidate.outputType = static_cast<ValueType>(constrain(
      preferences.getUChar("otype", TYPE_FLOAT), static_cast<int>(TYPE_FLOAT),
      static_cast<int>(TYPE_STRING)));
  candidate.clickMode = preferences.getUChar("cmode", MODE_PRESS_RELEASE) == MODE_SEQUENCE
                            ? MODE_SEQUENCE : MODE_PRESS_RELEASE;
  candidate.pressMessageCount = preferences.getUChar("pc", 0);
  candidate.releaseMessageCount = preferences.getUChar("rc", 0);
  if (candidate.displayName.isEmpty() || candidate.displayName.length() > 64 ||
      candidate.pressMessageCount + candidate.releaseMessageCount >
          MAX_KEY_OSC_MESSAGES) {
    preferences.end();
    return false;
  }
  for (uint8_t i = 0; i < candidate.pressMessageCount; ++i) {
    candidate.pressMessages[i].address =
        preferences.getString(indexedKey("pa", i).c_str(), "");
    candidate.pressMessages[i].valueStr =
        preferences.getString(indexedKey("pv", i).c_str(), "");
    candidate.pressMessages[i].valueType = static_cast<ValueType>(constrain(
        preferences.getUChar(indexedKey("pt", i).c_str(), TYPE_FLOAT),
        static_cast<int>(TYPE_FLOAT), static_cast<int>(TYPE_STRING)));
  }
  for (uint8_t i = 0; i < candidate.releaseMessageCount; ++i) {
    candidate.releaseMessages[i].address =
        preferences.getString(indexedKey("ra", i).c_str(), "");
    candidate.releaseMessages[i].valueStr =
        preferences.getString(indexedKey("rv", i).c_str(), "");
    candidate.releaseMessages[i].valueType = static_cast<ValueType>(constrain(
        preferences.getUChar(indexedKey("rt", i).c_str(), TYPE_FLOAT),
        static_cast<int>(TYPE_FLOAT), static_cast<int>(TYPE_STRING)));
  }
  candidate.clickSequence.address = preferences.getString("sa", "");
  candidate.clickSequence.valueType = static_cast<ValueType>(constrain(
      preferences.getUChar("stype", TYPE_FLOAT), static_cast<int>(TYPE_FLOAT),
      static_cast<int>(TYPE_STRING)));
  candidate.clickSequence.start = preferences.getFloat("start", 0);
  candidate.clickSequence.end = preferences.getFloat("end", 10);
  candidate.clickSequence.step = preferences.getFloat("step", 1);
  preferences.end();
  if (!validAddress(candidate.rotationAddress) ||
      !validAddress(candidate.clickSequence.address))
    return false;
  for (uint8_t i = 0; i < candidate.pressMessageCount; ++i)
    if (!validAddress(candidate.pressMessages[i].address)) return false;
  for (uint8_t i = 0; i < candidate.releaseMessageCount; ++i)
    if (!validAddress(candidate.releaseMessages[i].address)) return false;
  keySettingsNormalizeSequence(candidate.clickSequence);
  setting = candidate;
  return true;
}

bool writeSetting(const EncoderSetting& setting) {
  Preferences preferences;
  const String nameSpace = deviceNamespace(setting.identity);
  if (!preferences.begin(nameSpace.c_str(), false)) return false;
  preferences.putString("ver", STORAGE_VERSION);
  preferences.putString("id", setting.identity);
  preferences.putString("name", setting.displayName);
  preferences.putString("rot", setting.rotationAddress);
  preferences.putBool("inc", setting.sendIncrement);
  preferences.putFloat("amin", setting.absoluteInputMin);
  preferences.putFloat("amax", setting.absoluteInputMax);
  preferences.putFloat("iscale", setting.incrementScale);
  preferences.putFloat("omin", setting.outputMin);
  preferences.putFloat("omax", setting.outputMax);
  preferences.putUChar("otype", static_cast<uint8_t>(setting.outputType));
  preferences.putUChar("cmode", static_cast<uint8_t>(setting.clickMode));
  preferences.putUChar("pc", setting.pressMessageCount);
  preferences.putUChar("rc", setting.releaseMessageCount);
  for (uint8_t i = 0; i < setting.pressMessageCount; ++i) {
    preferences.putString(indexedKey("pa", i).c_str(),
                          setting.pressMessages[i].address);
    preferences.putString(indexedKey("pv", i).c_str(),
                          setting.pressMessages[i].valueStr);
    preferences.putUChar(indexedKey("pt", i).c_str(),
                         static_cast<uint8_t>(setting.pressMessages[i].valueType));
  }
  for (uint8_t i = 0; i < setting.releaseMessageCount; ++i) {
    preferences.putString(indexedKey("ra", i).c_str(),
                          setting.releaseMessages[i].address);
    preferences.putString(indexedKey("rv", i).c_str(),
                          setting.releaseMessages[i].valueStr);
    preferences.putUChar(indexedKey("rt", i).c_str(),
                         static_cast<uint8_t>(setting.releaseMessages[i].valueType));
  }
  preferences.putString("sa", setting.clickSequence.address);
  preferences.putUChar("stype",
                       static_cast<uint8_t>(setting.clickSequence.valueType));
  preferences.putFloat("start", setting.clickSequence.start);
  preferences.putFloat("end", setting.clickSequence.end);
  preferences.putFloat("step", setting.clickSequence.step);
  preferences.end();

  EncoderSetting verify = setting;
  bool found = false;
  const bool loaded = loadSetting(setting.identity, verify, found);
  return loaded && found && sameSetting(setting, verify);
}

void saveKnownDevices() {
  if (loadingKnown) return;
  String known;
  for (size_t i = 0; i < settingCount; ++i) {
    if (!known.isEmpty()) known += '\n';
    known += settings[i].identity;
  }
  Preferences preferences;
  if (preferences.begin("enccfg", false)) {
    preferences.putString("known", known);
    preferences.end();
  }
}

}  // namespace

void encoderSettingsSetup() {
  Preferences preferences;
  String known;
  if (preferences.begin("enccfg", true)) {
    known = preferences.getString("known", "");
    preferences.end();
  }
  loadingKnown = true;
  int offset = 0;
  while (offset < static_cast<int>(known.length())) {
    int end = known.indexOf('\n', offset);
    if (end < 0) end = known.length();
    const String identity = known.substring(offset, end);
    if (identity.startsWith("chain:") && identity.length() > 6) {
      const String uid = identity.substring(6);
      encoderSettingsEnsure(identity, String("Chain Encoder ") + uid);
    }
    offset = end + 1;
  }
  loadingKnown = false;
  Serial.printf("[ChainOSCmini][ENCCFG] setup_complete settings=%u\n",
                static_cast<unsigned>(settingCount));
}

EncoderSetting* encoderSettingsEnsure(const String& identity,
                                      const String& defaultName) {
  for (size_t i = 0; i < settingCount; ++i)
    if (settings[i].identity == identity) return &settings[i];
  if (settingCount >= MAX_ENCODER_SETTINGS) return nullptr;
  EncoderSetting& setting = settings[settingCount++];
  setting.identity = identity;
  setting.displayName = defaultName;
  setting.pressMessages[0].address = "/avatar/parameters/EncoderClick";
  setting.pressMessages[0].valueStr = "1.0";
  setting.pressMessages[0].valueType = TYPE_FLOAT;
  setting.releaseMessages[0].address = "/avatar/parameters/EncoderClick";
  setting.releaseMessages[0].valueStr = "0.0";
  setting.releaseMessages[0].valueType = TYPE_FLOAT;
  setting.clickSequence.address = "/avatar/parameters/EncoderSeq";
  keySettingsNormalizeSequence(setting.clickSequence);
  bool found = false;
  loadSetting(identity, setting, found);
  saveKnownDevices();
  return &setting;
}

size_t encoderSettingsCount() { return settingCount; }

EncoderSetting* encoderSettingsAt(size_t index) {
  return index < settingCount ? &settings[index] : nullptr;
}

bool encoderSettingsSave(const EncoderSetting& candidate) {
  if (candidate.identity.isEmpty() || candidate.displayName.isEmpty() ||
      candidate.displayName.length() > 64 ||
      !validAddress(candidate.rotationAddress) ||
      !validAddress(candidate.clickSequence.address) ||
      candidate.pressMessageCount + candidate.releaseMessageCount >
          MAX_KEY_OSC_MESSAGES || !isfinite(candidate.absoluteInputMin) ||
      !isfinite(candidate.absoluteInputMax) ||
      !isfinite(candidate.incrementScale) || !isfinite(candidate.outputMin) ||
      !isfinite(candidate.outputMax))
    return false;
  for (uint8_t i = 0; i < candidate.pressMessageCount; ++i)
    if (!validAddress(candidate.pressMessages[i].address)) return false;
  for (uint8_t i = 0; i < candidate.releaseMessageCount; ++i)
    if (!validAddress(candidate.releaseMessages[i].address)) return false;
  EncoderSetting* destination = nullptr;
  for (size_t i = 0; i < settingCount; ++i)
    if (settings[i].identity == candidate.identity) destination = &settings[i];
  if (!destination || !writeSetting(candidate)) return false;
  const uint8_t portMask = destination->connectedPortMask;
  *destination = candidate;
  destination->connectedPortMask = portMask;
  keySettingsNormalizeSequence(destination->clickSequence);
  Serial.printf("[ChainOSCmini][ENCCFG] saved identity=%s mode=%u press=%u release=%u\n",
                candidate.identity.c_str(),
                static_cast<unsigned>(candidate.clickMode),
                static_cast<unsigned>(candidate.pressMessageCount),
                static_cast<unsigned>(candidate.releaseMessageCount));
  return true;
}

bool encoderSettingsDelete(const String& identity) {
  size_t found = settingCount;
  for (size_t i = 0; i < settingCount; ++i) {
    if (settings[i].identity == identity) {
      if (settings[i].connectedPortMask != 0) return false;
      found = i;
      break;
    }
  }
  if (found == settingCount) return false;
  Preferences preferences;
  const String nameSpace = deviceNamespace(identity);
  if (preferences.begin(nameSpace.c_str(), false)) {
    preferences.clear();
    preferences.end();
  }
  for (size_t i = found + 1; i < settingCount; ++i)
    settings[i - 1] = settings[i];
  --settingCount;
  settings[settingCount] = EncoderSetting();
  saveKnownDevices();
  return true;
}

void encoderSettingsBeginPortUpdate(uint8_t portMask) {
  for (size_t i = 0; i < settingCount; ++i)
    settings[i].connectedPortMask &= ~portMask;
}

void encoderSettingsMarkConnected(const String& identity, uint8_t portMask) {
  for (size_t i = 0; i < settingCount; ++i)
    if (settings[i].identity == identity) {
      settings[i].connectedPortMask |= portMask;
      return;
    }
}
