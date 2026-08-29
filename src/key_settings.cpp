#include "key_settings.h"

#include <Preferences.h>
#include <ctype.h>
#include <math.h>

#include "device_file_storage.h"

namespace {
constexpr size_t MAX_SAVED_KEY_SETTINGS = 40;
constexpr size_t MAX_KEY_SETTINGS = MAX_SAVED_KEY_SETTINGS + 3;
constexpr char FIELD_SEPARATOR = '\x1f';
constexpr char LEGACY_STORAGE_VERSION[] = "K2";
constexpr char BASE_STORAGE_VERSION[] = "KC4";
constexpr char MESSAGE_STORAGE_VERSION[] = "KM3";
constexpr char DIRECT_STORAGE_VERSION[] = "N5";
KeySetting settings[MAX_KEY_SETTINGS];
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

String storageKey(const String& identity) {
  char key[11];
  snprintf(key, sizeof(key), "k%08X", static_cast<unsigned>(identityHash(identity)));
  return String(key);
}

String deviceNamespace(const String& identity) {
  char name[11];
  snprintf(name, sizeof(name), "s%08X",
           static_cast<unsigned>(identityHash(identity)));
  return String(name);
}

String cleanField(String value) {
  value.replace(String(FIELD_SEPARATOR), " ");
  return value;
}

String nextField(const String& source, int& offset) {
  if (offset > static_cast<int>(source.length())) return String();
  int end = source.indexOf(FIELD_SEPARATOR, offset);
  if (end < 0) end = source.length();
  const String value = source.substring(offset, end);
  offset = end + 1;
  return value;
}

void appendField(String& data, const String& value) {
  if (!data.isEmpty()) data += FIELD_SEPARATOR;
  data += cleanField(value);
}

void appendField(String& data, int value) {
  appendField(data, String(value));
}

void appendField(String& data, float value) {
  appendField(data, String(value, 7));
}

bool validAddress(const String& address) {
  if (address.isEmpty() || address.length() > 192 || address[0] != '/') return false;
  for (size_t i = 0; i < address.length(); ++i) {
    const char c = address[i];
    if (isspace(static_cast<unsigned char>(c)) || c == '#' || c == '*' ||
        c == ',' || c == '?' || c == '[' || c == ']' || c == '{' || c == '}')
      return false;
  }
  return true;
}

void sanitizeStoredMessages(KeyOscMessage* messages, uint8_t& count,
                            const String& identity, const char* eventName) {
  uint8_t writeIndex = 0;
  for (uint8_t readIndex = 0;
       readIndex < count && readIndex < MAX_KEY_OSC_MESSAGES; ++readIndex) {
    KeyOscMessage message = messages[readIndex];
    message.address.trim();
    bool reversed = false;
    if (!validAddress(message.address) && validAddress(message.valueStr)) {
      const String oldAddress = message.address;
      message.address = message.valueStr;
      message.valueStr = oldAddress;
      reversed = true;
    }
    if (!validAddress(message.address)) {
      Serial.printf("[ChainOSCmini][KEYCFG] message_dropped identity=%s event=%s index=%u address_bytes=%u value_bytes=%u\n",
                    identity.c_str(), eventName,
                    static_cast<unsigned>(readIndex),
                    static_cast<unsigned>(message.address.length()),
                    static_cast<unsigned>(message.valueStr.length()));
      continue;
    }
    if (reversed) {
      Serial.printf("[ChainOSCmini][KEYCFG] message_recovered identity=%s event=%s index=%u reason=address_value_reversed\n",
                    identity.c_str(), eventName,
                    static_cast<unsigned>(readIndex));
    }
    messages[writeIndex++] = message;
  }
  count = writeIndex;
}

String serializeBaseSetting(const KeySetting& setting) {
  String data;
  data.reserve(192);
  appendField(data, String(BASE_STORAGE_VERSION));
  appendField(data, setting.identity);
  appendField(data, setting.displayName);
  appendField(data, static_cast<int>(setting.mode));
  const KeyOscMessage defaultPress = setting.pressMessageCount > 0
                                         ? setting.pressMessages[0]
                                         : KeyOscMessage();
  const KeyOscMessage defaultRelease = setting.releaseMessageCount > 0
                                           ? setting.releaseMessages[0]
                                           : KeyOscMessage();
  appendField(data, defaultPress.address);
  appendField(data, defaultPress.valueStr);
  appendField(data, static_cast<int>(defaultPress.valueType));
  appendField(data, defaultRelease.address);
  appendField(data, defaultRelease.valueStr);
  appendField(data, static_cast<int>(defaultRelease.valueType));
  appendField(data, setting.sequence.address);
  appendField(data, static_cast<int>(setting.sequence.valueType));
  appendField(data, setting.sequence.start);
  appendField(data, setting.sequence.end);
  appendField(data, setting.sequence.step);
  return data;
}

String serializeKeyMessages(const KeySetting& setting) {
  String data;
  data.reserve(384);
  appendField(data, String(MESSAGE_STORAGE_VERSION));
  appendField(data, static_cast<int>(setting.pressMessageCount));
  for (uint8_t i = 0; i < setting.pressMessageCount; ++i) {
    appendField(data, setting.pressMessages[i].address);
    appendField(data, setting.pressMessages[i].valueStr);
    appendField(data, static_cast<int>(setting.pressMessages[i].valueType));
  }
  appendField(data, static_cast<int>(setting.releaseMessageCount));
  for (uint8_t i = 0; i < setting.releaseMessageCount; ++i) {
    appendField(data, setting.releaseMessages[i].address);
    appendField(data, setting.releaseMessages[i].valueStr);
    appendField(data, static_cast<int>(setting.releaseMessages[i].valueType));
  }
  return data;
}

bool applyBaseSetting(KeySetting& setting, const String& data,
                      const char*& failureReason) {
  int offset = 0;
  if (nextField(data, offset) != BASE_STORAGE_VERSION) {
    failureReason = "invalid_base_marker";
    return false;
  }
  if (nextField(data, offset) != setting.identity) {
    failureReason = "identity_mismatch";
    return false;
  }
  setting.displayName = nextField(data, offset);
  if (setting.displayName.isEmpty() || setting.displayName.length() > 64) {
    failureReason = "invalid_display_name";
    return false;
  }
  setting.mode = nextField(data, offset).toInt() == MODE_SEQUENCE
                     ? MODE_SEQUENCE : MODE_PRESS_RELEASE;
  setting.pressMessageCount = 1;
  setting.pressMessages[0].address = nextField(data, offset);
  setting.pressMessages[0].valueStr = nextField(data, offset);
  setting.pressMessages[0].valueType = static_cast<ValueType>(constrain(
      nextField(data, offset).toInt(), static_cast<int>(TYPE_FLOAT),
      static_cast<int>(TYPE_STRING)));
  setting.releaseMessageCount = 1;
  setting.releaseMessages[0].address = nextField(data, offset);
  setting.releaseMessages[0].valueStr = nextField(data, offset);
  setting.releaseMessages[0].valueType = static_cast<ValueType>(constrain(
      nextField(data, offset).toInt(), static_cast<int>(TYPE_FLOAT),
      static_cast<int>(TYPE_STRING)));
  setting.sequence.address = nextField(data, offset);
  setting.sequence.valueType = static_cast<ValueType>(constrain(
      nextField(data, offset).toInt(), static_cast<int>(TYPE_FLOAT),
      static_cast<int>(TYPE_STRING)));
  setting.sequence.start = nextField(data, offset).toFloat();
  setting.sequence.end = nextField(data, offset).toFloat();
  setting.sequence.step = nextField(data, offset).toFloat();
  if (!validAddress(setting.sequence.address)) {
    failureReason = "invalid_sequence_address";
    return false;
  }
  sanitizeStoredMessages(setting.pressMessages, setting.pressMessageCount,
                         setting.identity, "base_press");
  sanitizeStoredMessages(setting.releaseMessages, setting.releaseMessageCount,
                         setting.identity, "base_release");
  keySettingsNormalizeSequence(setting.sequence);
  failureReason = "none";
  return true;
}

bool applyKeyMessages(KeySetting& setting, const String& data,
                      const char*& failureReason) {
  int offset = 0;
  if (nextField(data, offset) != MESSAGE_STORAGE_VERSION) {
    failureReason = "invalid_message_marker";
    return false;
  }
  const int pressCount = nextField(data, offset).toInt();
  if (pressCount < 0 || pressCount > MAX_KEY_OSC_MESSAGES) {
    failureReason = "invalid_press_count";
    return false;
  }
  setting.pressMessageCount = static_cast<uint8_t>(pressCount);
  for (uint8_t i = 0; i < setting.pressMessageCount; ++i) {
    KeyOscMessage& message = setting.pressMessages[i];
    message.address = nextField(data, offset);
    message.valueStr = nextField(data, offset);
    message.valueType = static_cast<ValueType>(constrain(
        nextField(data, offset).toInt(), static_cast<int>(TYPE_FLOAT),
        static_cast<int>(TYPE_STRING)));
  }
  const int releaseCount = nextField(data, offset).toInt();
  if (releaseCount < 0 ||
      pressCount + releaseCount > MAX_KEY_OSC_MESSAGES) {
    failureReason = "invalid_release_count";
    return false;
  }
  setting.releaseMessageCount = static_cast<uint8_t>(releaseCount);
  for (uint8_t i = 0; i < setting.releaseMessageCount; ++i) {
    KeyOscMessage& message = setting.releaseMessages[i];
    message.address = nextField(data, offset);
    message.valueStr = nextField(data, offset);
    message.valueType = static_cast<ValueType>(constrain(
        nextField(data, offset).toInt(), static_cast<int>(TYPE_FLOAT),
        static_cast<int>(TYPE_STRING)));
  }
  sanitizeStoredMessages(setting.pressMessages, setting.pressMessageCount,
                         setting.identity, "press");
  sanitizeStoredMessages(setting.releaseMessages, setting.releaseMessageCount,
                         setting.identity, "release");
  failureReason = "none";
  return true;
}

bool sameMessage(const KeyOscMessage& left, const KeyOscMessage& right) {
  return left.address == right.address && left.valueStr == right.valueStr &&
         left.valueType == right.valueType;
}

bool sameStoredSetting(const KeySetting& left, const KeySetting& right) {
  if (left.identity != right.identity || left.displayName != right.displayName ||
      left.mode != right.mode ||
      left.pressMessageCount != right.pressMessageCount ||
      left.releaseMessageCount != right.releaseMessageCount ||
      left.sequence.address != right.sequence.address ||
      left.sequence.valueType != right.sequence.valueType ||
      fabsf(left.sequence.start - right.sequence.start) > 0.00001f ||
      fabsf(left.sequence.end - right.sequence.end) > 0.00001f ||
      fabsf(left.sequence.step - right.sequence.step) > 0.00001f) return false;
  for (uint8_t i = 0; i < left.pressMessageCount; ++i)
    if (!sameMessage(left.pressMessages[i], right.pressMessages[i])) return false;
  for (uint8_t i = 0; i < left.releaseMessageCount; ++i)
    if (!sameMessage(left.releaseMessages[i], right.releaseMessages[i])) return false;
  return true;
}

String indexedKey(const char* prefix, uint8_t index) {
  return String(prefix) + String(static_cast<unsigned>(index));
}

bool loadDirectSetting(const String& identity, KeySetting& setting,
                       bool& found, const char*& failureReason) {
  found = false;
  Preferences preferences;
  const String nameSpace = deviceNamespace(identity);
  if (!preferences.begin(nameSpace.c_str(), true)) {
    failureReason = "direct_nvs_open_failed";
    return false;
  }
  const String version = preferences.getString("ver", "");
  if (version != DIRECT_STORAGE_VERSION) {
    preferences.end();
    failureReason = "direct_not_found";
    return false;
  }
  found = true;
  KeySetting candidate = setting;
  if (preferences.getString("id", "") != identity) {
    preferences.end();
    failureReason = "direct_identity_mismatch";
    return false;
  }
  candidate.displayName = preferences.getString("name", "");
  candidate.mode = preferences.getUChar("mode", MODE_PRESS_RELEASE) == MODE_SEQUENCE
                       ? MODE_SEQUENCE : MODE_PRESS_RELEASE;
  candidate.pressMessageCount = preferences.getUChar("pc", 0);
  candidate.releaseMessageCount = preferences.getUChar("rc", 0);
  if (candidate.displayName.isEmpty() || candidate.displayName.length() > 64 ||
      candidate.pressMessageCount + candidate.releaseMessageCount >
          MAX_KEY_OSC_MESSAGES) {
    preferences.end();
    failureReason = "direct_invalid_header";
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
    if (!validAddress(candidate.pressMessages[i].address)) {
      preferences.end();
      failureReason = "direct_invalid_press_address";
      return false;
    }
  }
  for (uint8_t i = 0; i < candidate.releaseMessageCount; ++i) {
    candidate.releaseMessages[i].address =
        preferences.getString(indexedKey("ra", i).c_str(), "");
    candidate.releaseMessages[i].valueStr =
        preferences.getString(indexedKey("rv", i).c_str(), "");
    candidate.releaseMessages[i].valueType = static_cast<ValueType>(constrain(
        preferences.getUChar(indexedKey("rt", i).c_str(), TYPE_FLOAT),
        static_cast<int>(TYPE_FLOAT), static_cast<int>(TYPE_STRING)));
    if (!validAddress(candidate.releaseMessages[i].address)) {
      preferences.end();
      failureReason = "direct_invalid_release_address";
      return false;
    }
  }
  candidate.sequence.address = preferences.getString("sa", "");
  candidate.sequence.valueType = static_cast<ValueType>(constrain(
      preferences.getUChar("stype", TYPE_FLOAT), static_cast<int>(TYPE_FLOAT),
      static_cast<int>(TYPE_STRING)));
  candidate.sequence.start = preferences.getFloat("start", 0);
  candidate.sequence.end = preferences.getFloat("end", 10);
  candidate.sequence.step = preferences.getFloat("step", 1);
  preferences.end();
  if (!validAddress(candidate.sequence.address)) {
    failureReason = "direct_invalid_sequence_address";
    return false;
  }
  keySettingsNormalizeSequence(candidate.sequence);
  setting = candidate;
  failureReason = "none";
  return true;
}

bool saveDirectSetting(const KeySetting& setting) {
  if (!deviceFileStorageSave(setting)) return false;
  KeySetting verify = setting;
  const bool match = deviceFileStorageLoad(verify) ==
                         DeviceFileLoadResult::Loaded &&
                     sameStoredSetting(setting, verify);
  Serial.printf("[ChainOSCmini][KEYCFG] file_verify identity=%s match=%u\n",
                setting.identity.c_str(), match ? 1U : 0U);
  return match;
}

bool parseStoredMessageCount(const String& field, uint8_t& count) {
  // Early K2 builds serialized uint8_t with String(uint8_t), which can store
  // the numeric value as a single control byte instead of ASCII. Accept both
  // representations so settings already written by those builds survive.
  if (field.length() == 1) {
    const uint8_t raw = static_cast<uint8_t>(field[0]);
    if (raw <= MAX_KEY_OSC_MESSAGES) {
      count = raw;
      return true;
    }
  }
  if (field.isEmpty()) {
    count = 0;
    return true;
  }
  for (size_t i = 0; i < field.length(); ++i)
    if (!isdigit(static_cast<unsigned char>(field[i]))) return false;
  const int parsed = field.toInt();
  if (parsed < 0 || parsed > MAX_KEY_OSC_MESSAGES) return false;
  count = static_cast<uint8_t>(parsed);
  return true;
}

bool loadV2(KeySetting& setting, const String& data, int offset,
            const char*& failureReason) {
  const String defaultSequenceAddress = setting.sequence.address;
  if (nextField(data, offset) != setting.identity) {
    failureReason = "identity_mismatch";
    return false;
  }
  setting.displayName = nextField(data, offset);
  if (setting.displayName.isEmpty() || setting.displayName.length() > 64) {
    failureReason = "invalid_display_name";
    return false;
  }
  setting.mode = nextField(data, offset).toInt() == MODE_SEQUENCE
                     ? MODE_SEQUENCE : MODE_PRESS_RELEASE;
  const String pressCountField = nextField(data, offset);
  const String releaseCountField = nextField(data, offset);
  if (!parseStoredMessageCount(pressCountField, setting.pressMessageCount) ||
      !parseStoredMessageCount(releaseCountField, setting.releaseMessageCount)) {
    failureReason = "invalid_message_count";
    return false;
  }
  if (setting.pressMessageCount + setting.releaseMessageCount > MAX_KEY_OSC_MESSAGES) {
    failureReason = "message_count_over_limit";
    return false;
  }
  for (uint8_t i = 0; i < setting.pressMessageCount; ++i) {
    KeyOscMessage& message = setting.pressMessages[i];
    message.address = nextField(data, offset);
    message.valueType = static_cast<ValueType>(constrain(
        nextField(data, offset).toInt(), static_cast<int>(TYPE_FLOAT),
        static_cast<int>(TYPE_STRING)));
    message.valueStr = nextField(data, offset);
    if (!validAddress(message.address)) {
      failureReason = "invalid_press_address";
      return false;
    }
  }
  for (uint8_t i = 0; i < setting.releaseMessageCount; ++i) {
    KeyOscMessage& message = setting.releaseMessages[i];
    message.address = nextField(data, offset);
    message.valueType = static_cast<ValueType>(constrain(
        nextField(data, offset).toInt(), static_cast<int>(TYPE_FLOAT),
        static_cast<int>(TYPE_STRING)));
    message.valueStr = nextField(data, offset);
    if (!validAddress(message.address)) {
      failureReason = "invalid_release_address";
      return false;
    }
  }
  setting.sequence.address = nextField(data, offset);
  setting.sequence.start = nextField(data, offset).toFloat();
  setting.sequence.end = nextField(data, offset).toFloat();
  setting.sequence.step = nextField(data, offset).toFloat();
  setting.sequence.valueType = static_cast<ValueType>(constrain(
      nextField(data, offset).toInt(), static_cast<int>(TYPE_FLOAT),
      static_cast<int>(TYPE_STRING)));
  if (!validAddress(setting.sequence.address)) {
    Serial.printf("[ChainOSCmini][KEYCFG] load_sequence_invalid identity=%s mode=%u address_bytes=%u offset=%d data_bytes=%u\n",
                  setting.identity.c_str(), static_cast<unsigned>(setting.mode),
                  static_cast<unsigned>(setting.sequence.address.length()), offset,
                  static_cast<unsigned>(data.length()));
    // An unused Sequence field must not invalidate valid Press / Release
    // settings. Recover a safe Sequence address so the mode can be selected
    // later without losing the active configuration.
    if (setting.mode == MODE_PRESS_RELEASE) {
      if (setting.pressMessageCount > 0 &&
          validAddress(setting.pressMessages[0].address)) {
        setting.sequence.address = setting.pressMessages[0].address;
      } else if (setting.releaseMessageCount > 0 &&
                 validAddress(setting.releaseMessages[0].address)) {
        setting.sequence.address = setting.releaseMessages[0].address;
      } else {
        setting.sequence.address = defaultSequenceAddress;
      }
      if (!validAddress(setting.sequence.address)) {
        failureReason = "invalid_sequence_address_no_fallback";
        return false;
      }
      Serial.printf("[ChainOSCmini][KEYCFG] load_recovered identity=%s field=sequence_address fallback_bytes=%u\n",
                    setting.identity.c_str(),
                    static_cast<unsigned>(setting.sequence.address.length()));
    } else {
      failureReason = "invalid_sequence_address";
      return false;
    }
  }
  keySettingsNormalizeSequence(setting.sequence);
  failureReason = "none";
  return true;
}

bool loadLegacySetting(KeySetting& setting) {
  KeySetting directCandidate = setting;
  bool directFound = false;
  const char* directReason = "unknown";
  if (loadDirectSetting(setting.identity, directCandidate, directFound,
                        directReason)) {
    setting = directCandidate;
    Serial.printf("[ChainOSCmini][KEYCFG] load_ok identity=%s format=%s mode=%u press=%u release=%u namespace=%s\n",
                  setting.identity.c_str(), DIRECT_STORAGE_VERSION,
                  static_cast<unsigned>(setting.mode),
                  static_cast<unsigned>(setting.pressMessageCount),
                  static_cast<unsigned>(setting.releaseMessageCount),
                  deviceNamespace(setting.identity).c_str());
    return true;
  }
  if (directFound) {
    Serial.printf("[ChainOSCmini][KEYCFG] load_failed identity=%s format=%s reason=%s\n",
                  setting.identity.c_str(), DIRECT_STORAGE_VERSION,
                  directReason);
    return false;
  }
  const String key = storageKey(setting.identity);
  Serial.printf("[ChainOSCmini][KEYCFG] load_begin identity=%s key=%s\n",
                setting.identity.c_str(), key.c_str());
  Preferences preferences;
  if (!preferences.begin("keycfg", true)) {
    Serial.printf("[ChainOSCmini][KEYCFG] load_failed identity=%s reason=nvs_open_failed\n",
                  setting.identity.c_str());
    return false;
  }
  const String data = preferences.getString(key.c_str(), "");
  preferences.end();
  Serial.printf("[ChainOSCmini][KEYCFG] load_data identity=%s bytes=%u\n",
                setting.identity.c_str(), static_cast<unsigned>(data.length()));
  if (data.isEmpty()) {
    Serial.printf("[ChainOSCmini][KEYCFG] load_default identity=%s reason=no_saved_data\n",
                  setting.identity.c_str());
    return false;
  }
  int offset = 0;
  const String first = nextField(data, offset);
  KeySetting candidate = setting;
  if (first == BASE_STORAGE_VERSION) {
    const char* failureReason = "unknown";
    if (!applyBaseSetting(candidate, data, failureReason)) {
      Serial.printf("[ChainOSCmini][KEYCFG] load_failed identity=%s format=%s reason=%s bytes=%u\n",
                    setting.identity.c_str(), BASE_STORAGE_VERSION,
                    failureReason, static_cast<unsigned>(data.length()));
      return false;
    }
    // Match M5ChainOSC: the base configuration remains valid even when the
    // separately stored multi-message extension cannot be applied.
    setting = candidate;
    Preferences multiPreferences;
    if (!multiPreferences.begin("keymulti", true)) {
      Serial.printf("[ChainOSCmini][KEYCFG] load_failed identity=%s format=%s reason=message_nvs_open_failed\n",
                    setting.identity.c_str(), BASE_STORAGE_VERSION);
      return true;
    }
    const String messages = multiPreferences.getString(key.c_str(), "");
    multiPreferences.end();
    if (!applyKeyMessages(candidate, messages, failureReason)) {
      Serial.printf("[ChainOSCmini][KEYCFG] load_failed identity=%s format=%s reason=%s base_bytes=%u message_bytes=%u\n",
                    setting.identity.c_str(), BASE_STORAGE_VERSION,
                    failureReason, static_cast<unsigned>(data.length()),
                    static_cast<unsigned>(messages.length()));
      return true;
    }
    setting = candidate;
    Serial.printf("[ChainOSCmini][KEYCFG] load_ok identity=%s format=%s/%s mode=%u press=%u release=%u base_bytes=%u message_bytes=%u\n",
                  setting.identity.c_str(), BASE_STORAGE_VERSION,
                  MESSAGE_STORAGE_VERSION, static_cast<unsigned>(setting.mode),
                  static_cast<unsigned>(setting.pressMessageCount),
                  static_cast<unsigned>(setting.releaseMessageCount),
                  static_cast<unsigned>(data.length()),
                  static_cast<unsigned>(messages.length()));
    return true;
  }
  if (first == LEGACY_STORAGE_VERSION) {
    const char* failureReason = "unknown";
    if (!loadV2(candidate, data, offset, failureReason)) {
      Serial.printf("[ChainOSCmini][KEYCFG] load_failed identity=%s format=%s reason=%s bytes=%u\n",
                    setting.identity.c_str(), LEGACY_STORAGE_VERSION, failureReason,
                    static_cast<unsigned>(data.length()));
      return false;
    }
    setting = candidate;
    Serial.printf("[ChainOSCmini][KEYCFG] load_ok identity=%s format=%s mode=%u press=%u release=%u name_bytes=%u\n",
                  setting.identity.c_str(), LEGACY_STORAGE_VERSION,
                  static_cast<unsigned>(setting.mode),
                  static_cast<unsigned>(setting.pressMessageCount),
                  static_cast<unsigned>(setting.releaseMessageCount),
                  static_cast<unsigned>(setting.displayName.length()));
    return true;
  }

  // 0.8.0 layout: identity, name, address, press Int, release Int.
  if (first != setting.identity) {
    Serial.printf("[ChainOSCmini][KEYCFG] load_failed identity=%s format=legacy reason=identity_mismatch bytes=%u\n",
                  setting.identity.c_str(), static_cast<unsigned>(data.length()));
    return false;
  }
  candidate.displayName = nextField(data, offset);
  const String address = nextField(data, offset);
  if (candidate.displayName.isEmpty() || candidate.displayName.length() > 64) {
    Serial.printf("[ChainOSCmini][KEYCFG] load_failed identity=%s format=legacy reason=invalid_display_name\n",
                  setting.identity.c_str());
    return false;
  }
  if (!validAddress(address)) {
    Serial.printf("[ChainOSCmini][KEYCFG] load_failed identity=%s format=legacy reason=invalid_address\n",
                  setting.identity.c_str());
    return false;
  }
  candidate.mode = MODE_PRESS_RELEASE;
  candidate.pressMessageCount = candidate.releaseMessageCount = 1;
  candidate.pressMessages[0].address = address;
  candidate.pressMessages[0].valueType = TYPE_INT;
  candidate.pressMessages[0].valueStr = nextField(data, offset);
  candidate.releaseMessages[0].address = address;
  candidate.releaseMessages[0].valueType = TYPE_INT;
  candidate.releaseMessages[0].valueStr = nextField(data, offset);
  candidate.sequence.address = address;
  keySettingsNormalizeSequence(candidate.sequence);
  setting = candidate;
  Serial.printf("[ChainOSCmini][KEYCFG] load_ok identity=%s format=legacy mode=%u press=%u release=%u name_bytes=%u\n",
                setting.identity.c_str(), static_cast<unsigned>(setting.mode),
                static_cast<unsigned>(setting.pressMessageCount),
                static_cast<unsigned>(setting.releaseMessageCount),
                static_cast<unsigned>(setting.displayName.length()));
  return true;
}

bool loadSetting(KeySetting& setting) {
  const DeviceFileLoadResult result = deviceFileStorageLoad(setting);
  if (result == DeviceFileLoadResult::Loaded) {
    if (!validAddress(setting.sequence.address) ||
        setting.pressMessageCount + setting.releaseMessageCount >
            MAX_KEY_OSC_MESSAGES)
      return false;
    for (uint8_t i = 0; i < setting.pressMessageCount; ++i)
      if (!validAddress(setting.pressMessages[i].address)) return false;
    for (uint8_t i = 0; i < setting.releaseMessageCount; ++i)
      if (!validAddress(setting.releaseMessages[i].address)) return false;
    keySettingsNormalizeSequence(setting.sequence);
    return true;
  }
  if (result == DeviceFileLoadResult::Error) return false;
  if (!loadLegacySetting(setting)) return false;
  if (deviceFileStorageSave(setting)) {
    Serial.printf("[ChainOSCmini][KEYCFG] migrated identity=%s source=NVS target=LittleFS\n",
                  setting.identity.c_str());
    Preferences direct;
    const String nameSpace = deviceNamespace(setting.identity);
    if (direct.begin(nameSpace.c_str(), false)) {
      direct.clear();
      direct.end();
    }
    const String key = storageKey(setting.identity);
    Preferences base;
    if (base.begin("keycfg", false)) {
      base.remove(key.c_str());
      base.end();
    }
    Preferences messages;
    if (messages.begin("keymulti", false)) {
      messages.remove(key.c_str());
      messages.end();
    }
    Serial.printf("[ChainOSCmini][KEYCFG] legacy_removed identity=%s source=NVS\n",
                  setting.identity.c_str());
  } else {
    Serial.printf("[ChainOSCmini][KEYCFG] migration_failed identity=%s source=NVS target=LittleFS\n",
                  setting.identity.c_str());
  }
  return true;
}

void saveKnownDevices() {
  if (loadingKnown) return;
  String known;
  for (size_t i = 0; i < settingCount; ++i) {
    if (settings[i].builtIn) continue;
    if (!deviceFileStorageExists("key", settings[i].identity)) continue;
    if (!known.isEmpty()) known += '\n';
    known += settings[i].identity;
  }
  Preferences preferences;
  if (preferences.begin("keycfg", false)) {
    preferences.putString("known", known);
    preferences.end();
  }
}
}  // namespace

void keySettingsNormalizeSequence(KeySequenceConfig& sequence) {
  if (!isfinite(sequence.start)) sequence.start = 0;
  if (!isfinite(sequence.end)) sequence.end = 10;
  if (!isfinite(sequence.step) || fabsf(sequence.step) < 1e-9f) sequence.step = 1;
  if (sequence.start <= sequence.end && sequence.step < 0) sequence.step = -sequence.step;
  if (sequence.start > sequence.end && sequence.step > 0) sequence.step = -sequence.step;
  sequence.current = sequence.start;
}

void keySettingsSetup() {
  Serial.println("[ChainOSCmini][KEYCFG] setup_begin=true");
  deviceFileStorageBegin();
  keySettingsEnsure("dualkey:1", "DualKey KEY1", "/chainoscmini/dualkey/key1");
  keySettingsEnsure("dualkey:2", "DualKey KEY2", "/chainoscmini/dualkey/key2");
  String fileIdentities[MAX_SAVED_KEY_SETTINGS];
  const size_t fileCount =
      deviceFileStorageList("key", fileIdentities, MAX_SAVED_KEY_SETTINGS);
  loadingKnown = true;
  for (size_t i = 0; i < fileCount; ++i) {
    const String& identity = fileIdentities[i];
    if (identity.startsWith("chain:") && identity.length() > 6) {
      const String uid = identity.substring(6);
      keySettingsEnsure(identity, String("Chain Key ") + uid,
                        String("/chainoscmini/chain/key/") + uid);
    }
  }
  loadingKnown = false;
  Preferences preferences;
  String known;
  if (preferences.begin("keycfg", true)) {
    known = preferences.getString("known", "");
    preferences.end();
  }
  Serial.printf("[ChainOSCmini][KEYCFG] known_devices bytes=%u\n",
                static_cast<unsigned>(known.length()));
  loadingKnown = true;
  int offset = 0;
  while (offset < static_cast<int>(known.length())) {
    int end = known.indexOf('\n', offset);
    if (end < 0) end = known.length();
    const String identity = known.substring(offset, end);
    if (identity.startsWith("chain:") && identity.length() > 6) {
      const String uid = identity.substring(6);
      keySettingsEnsure(identity, String("Chain Key ") + uid,
                        String("/chainoscmini/chain/key/") + uid);
    }
    offset = end + 1;
  }
  loadingKnown = false;
  Serial.printf("[ChainOSCmini][KEYCFG] setup_complete settings=%u\n",
                static_cast<unsigned>(settingCount));
}

KeySetting* keySettingsEnsure(const String& identity, const String& defaultName,
                              const String& defaultAddress) {
  for (size_t i = 0; i < settingCount; ++i)
    if (settings[i].identity == identity) return &settings[i];
  if (settingCount >= MAX_KEY_SETTINGS) return nullptr;
  KeySetting& setting = settings[settingCount++];
  setting.identity = identity;
  setting.displayName = defaultName;
  setting.builtIn = identity.startsWith("dualkey:");
  setting.pressMessages[0].address = defaultAddress;
  setting.pressMessages[0].valueType = TYPE_INT;
  setting.pressMessages[0].valueStr = "1";
  setting.releaseMessages[0].address = defaultAddress;
  setting.releaseMessages[0].valueType = TYPE_INT;
  setting.releaseMessages[0].valueStr = "0";
  setting.sequence.address = defaultAddress;
  keySettingsNormalizeSequence(setting.sequence);
  loadSetting(setting);
  return &setting;
}

size_t keySettingsCount() { return settingCount; }
KeySetting* keySettingsAt(size_t index) {
  return index < settingCount ? &settings[index] : nullptr;
}

bool keySettingsSave(const KeySetting& candidate) {
  if (candidate.identity.isEmpty() || candidate.displayName.isEmpty() ||
      candidate.displayName.length() > 64 ||
      candidate.pressMessageCount + candidate.releaseMessageCount > MAX_KEY_OSC_MESSAGES ||
      !validAddress(candidate.sequence.address)) return false;
  for (uint8_t i = 0; i < candidate.pressMessageCount; ++i)
    if (!validAddress(candidate.pressMessages[i].address)) return false;
  for (uint8_t i = 0; i < candidate.releaseMessageCount; ++i)
    if (!validAddress(candidate.releaseMessages[i].address)) return false;

  KeySetting* setting = nullptr;
  for (size_t i = 0; i < settingCount; ++i)
    if (settings[i].identity == candidate.identity) { setting = &settings[i]; break; }
  if (setting == nullptr) return false;

  if (!saveDirectSetting(candidate)) return false;
  const bool builtIn = setting->builtIn;
  const uint8_t portMask = setting->connectedPortMask;
  *setting = candidate;
  setting->builtIn = builtIn;
  setting->connectedPortMask = portMask;
  keySettingsNormalizeSequence(setting->sequence);
  Serial.printf("[ChainOSCmini][KEYCFG] saved identity=%s format=%s mode=%u press=%u release=%u sequence_address_bytes=%u namespace=%s\n",
                candidate.identity.c_str(), DIRECT_STORAGE_VERSION,
                static_cast<unsigned>(candidate.mode),
                static_cast<unsigned>(candidate.pressMessageCount),
                static_cast<unsigned>(candidate.releaseMessageCount),
                static_cast<unsigned>(candidate.sequence.address.length()),
                deviceNamespace(candidate.identity).c_str());
  return true;
}

bool keySettingsDelete(const String& identity) {
  if (identity.isEmpty()) return false;
  size_t found = settingCount;
  for (size_t i = 0; i < settingCount; ++i) {
    if (settings[i].identity == identity) {
      if (settings[i].builtIn || settings[i].connectedPortMask != 0) return false;
      found = i;
      break;
    }
  }
  if (found == settingCount) return false;

  if (!deviceFileStorageRemove("key", identity)) return false;

  Preferences directPreferences;
  const String directNamespace = deviceNamespace(identity);
  if (directPreferences.begin(directNamespace.c_str(), false)) {
    directPreferences.clear();
    directPreferences.end();
  }

  Preferences preferences;
  if (!preferences.begin("keycfg", false)) return false;
  const String key = storageKey(identity);
  preferences.remove(key.c_str());
  preferences.end();
  Preferences multiPreferences;
  if (multiPreferences.begin("keymulti", false)) {
    multiPreferences.remove(key.c_str());
    multiPreferences.end();
  }

  for (size_t i = found + 1; i < settingCount; ++i)
    settings[i - 1] = settings[i];
  --settingCount;
  settings[settingCount] = KeySetting();
  saveKnownDevices();
  Serial.printf("[ChainOSCmini][KEYCFG] deleted identity=%s\n", identity.c_str());
  return true;
}

void keySettingsBeginPortUpdate(uint8_t portMask) {
  for (size_t i = 0; i < settingCount; ++i)
    if (!settings[i].builtIn) settings[i].connectedPortMask &= ~portMask;
}

void keySettingsMarkConnected(const String& identity, uint8_t portMask) {
  for (size_t i = 0; i < settingCount; ++i)
    if (settings[i].identity == identity) { settings[i].connectedPortMask |= portMask; return; }
}

void keySettingsPrintState() {
  Serial.printf("[ChainOSCmini][KEYCFG_STATE] settings=%u\n",
                static_cast<unsigned>(settingCount));
  for (size_t i = 0; i < settingCount; ++i) {
    const KeySetting& setting = settings[i];
    const String pressAddress = setting.pressMessageCount > 0
                                    ? setting.pressMessages[0].address
                                    : String();
    const String releaseAddress = setting.releaseMessageCount > 0
                                      ? setting.releaseMessages[0].address
                                      : String();
    Serial.printf("[ChainOSCmini][KEYCFG_STATE] identity=%s name=%s mode=%u press=%u release=%u press0=%s release0=%s sequence=%s\n",
                  setting.identity.c_str(), setting.displayName.c_str(),
                  static_cast<unsigned>(setting.mode),
                  static_cast<unsigned>(setting.pressMessageCount),
                  static_cast<unsigned>(setting.releaseMessageCount),
                  pressAddress.c_str(), releaseAddress.c_str(),
                  setting.sequence.address.c_str());
  }
}
