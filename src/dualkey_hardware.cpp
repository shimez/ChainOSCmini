#include "dualkey_hardware.h"

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#include "config.h"
#include "osc_manager.h"

namespace {

struct DebouncedKey {
  uint8_t pin;
  const char* name;
  bool rawPressed;
  bool stablePressed;
  unsigned long changedAtMs;
};

Adafruit_NeoPixel keyLeds(LED_COUNT, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);
// The two WS2812 LEDs are wired in the opposite order to KEY1/KEY2.
// Keep the key/OSC numbering unchanged and translate only the LED index.
constexpr uint8_t KEY_LED_INDEX[] = {1, 0};
static_assert(LED_COUNT == 2, "DualKey LED mapping requires two LEDs");
DebouncedKey keys[] = {
    {KEY1_PIN, "KEY1", false, false, 0},
    {KEY2_PIN, "KEY2", false, false, 0},
};

NetworkLedState networkLedState = NetworkLedState::CONNECTING;
bool ledsReady = false;
unsigned long lastAnimationMs = 0;
unsigned long networkStateStartedAtMs = 0;
unsigned long activityEndsAtMs = 0;
unsigned long activityReadyAtMs = 0;
unsigned long identifyUntilMs[LED_COUNT] = {0, 0};

bool identifyActive(size_t index, unsigned long now) {
  return identifyUntilMs[index] != 0 &&
         static_cast<long>(identifyUntilMs[index] - now) > 0;
}

bool before(unsigned long now, unsigned long deadline) {
  return static_cast<long>(now - deadline) < 0;
}

bool networkLedOn(unsigned long now) {
  const unsigned long elapsed = now - networkStateStartedAtMs;
  if (networkLedState == NetworkLedState::CONNECTED) return true;
  if (networkLedState == NetworkLedState::AP_MODE)
    return elapsed % (STATUS_LED_AP_ON_MS + STATUS_LED_AP_OFF_MS) <
           STATUS_LED_AP_ON_MS;
  const unsigned long phase =
      elapsed % (STATUS_LED_CONNECTING_ON_MS * 2 +
                 STATUS_LED_CONNECTING_OFF_MS + STATUS_LED_CONNECTING_GAP_MS);
  return phase < STATUS_LED_CONNECTING_ON_MS ||
         (phase >= STATUS_LED_CONNECTING_ON_MS +
                       STATUS_LED_CONNECTING_OFF_MS &&
          phase < STATUS_LED_CONNECTING_ON_MS * 2 +
                      STATUS_LED_CONNECTING_OFF_MS);
}

uint32_t networkColor() {
  if (networkLedState == NetworkLedState::AP_MODE)
    return keyLeds.Color(16, 0, 0);
  if (networkLedState == NetworkLedState::CONNECTED)
    return keyLeds.Color(0, 16, 0);
  return keyLeds.Color(0, 0, 16);
}

uint32_t pressedColor() {
  return keyLeds.Color(64, 16, 0);
}

void renderLeds(unsigned long now) {
  const bool activityActive = before(now, activityEndsAtMs);
  const uint32_t background = !activityActive && networkLedOn(now)
                                  ? networkColor()
                                  : keyLeds.Color(0, 0, 0);
  for (size_t index = 0; index < LED_COUNT; ++index) {
    keyLeds.setPixelColor(
        KEY_LED_INDEX[index],
        identifyActive(index, now) ? pressedColor()
                                   : keys[index].stablePressed ? pressedColor()
                                                               : background);
  }
  keyLeds.show();
}

void updateKey(size_t index, unsigned long now) {
  DebouncedKey& key = keys[index];
  const bool pressed = digitalRead(key.pin) == LOW;

  if (pressed != key.rawPressed) {
    key.rawPressed = pressed;
    key.changedAtMs = now;
  }

  if (key.stablePressed == key.rawPressed ||
      now - key.changedAtMs < KEY_DEBOUNCE_MS) {
    return;
  }

  key.stablePressed = key.rawPressed;
  renderLeds(now);
  Serial.printf("[ChainOSCmini][KEY] %s=%s uptime=%lu ms\n", key.name,
                key.stablePressed ? "PRESSED" : "RELEASED",
                static_cast<unsigned long>(now));
  oscSendDualKey(static_cast<uint8_t>(index + 1), key.stablePressed);
}

}  // namespace

void dualKeyHardwareSetup() {
  pinMode(KEY1_PIN, INPUT_PULLUP);
  pinMode(KEY2_PIN, INPUT_PULLUP);

  pinMode(LED_POWER_PIN, OUTPUT);
  digitalWrite(LED_POWER_PIN, HIGH);
  delay(1);

  keyLeds.begin();
  keyLeds.clear();
  ledsReady = true;
  networkStateStartedAtMs = millis();

  for (size_t index = 0; index < LED_COUNT; ++index) {
    const bool pressed = digitalRead(keys[index].pin) == LOW;
    keys[index].rawPressed = pressed;
    keys[index].stablePressed = pressed;
    keys[index].changedAtMs = millis();
  }
  renderLeds(millis());

  Serial.println("[ChainOSCmini][GPIO] KEY1=GPIO0 KEY2=GPIO17 debounce=20ms");
  Serial.println("[ChainOSCmini][GPIO] WS2812=GPIO21 PWR_EN=GPIO40 count=2");
  Serial.println("[ChainOSCmini][GPIO] ap=RED_BLINK connecting=BLUE_DOUBLE_BLINK connected=GREEN pressed=ORANGE");
}

void dualKeyHardwareUpdate() {
  const unsigned long now = millis();
  updateKey(0, now);
  updateKey(1, now);
  bool identifyExpired = false;
  for (size_t index = 0; index < LED_COUNT; ++index) {
    if (identifyUntilMs[index] != 0 && !identifyActive(index, now)) {
      identifyUntilMs[index] = 0;
      identifyExpired = true;
    }
  }
  if (identifyExpired) renderLeds(now);
  dualKeyStatusLedUpdate();
}

void dualKeyStatusLedUpdate() {
  const unsigned long now = millis();
  if (!ledsReady || now - lastAnimationMs < STATUS_LED_UPDATE_INTERVAL_MS)
    return;
  lastAnimationMs = now;
  renderLeds(now);
}

void dualKeyNotifyOscTx() {
  const unsigned long now = millis();
  if (before(now, activityEndsAtMs) || before(now, activityReadyAtMs)) return;
  activityEndsAtMs = now + STATUS_LED_ACTIVITY_OFF_MS;
  activityReadyAtMs = activityEndsAtMs + STATUS_LED_ACTIVITY_BASE_GAP_MS;
  if (ledsReady) renderLeds(now);
}

void dualKeySetNetworkLedState(NetworkLedState state) {
  if (networkLedState == state) return;
  networkLedState = state;
  networkStateStartedAtMs = millis();
  if (ledsReady) renderLeds(networkStateStartedAtMs);
}

bool dualKeyIdentifyDevice(const String& identity) {
  size_t index = LED_COUNT;
  if (identity == "dualkey:1") index = 0;
  else if (identity == "dualkey:2") index = 1;
  if (index >= LED_COUNT || !ledsReady) return false;
  identifyUntilMs[index] = millis() + 10000UL;
  renderLeds(millis());
  return true;
}
