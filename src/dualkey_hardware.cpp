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

uint8_t pulseLevel(unsigned long now, uint8_t minimum, uint8_t maximum) {
  constexpr unsigned long kPulsePeriodMs = 2000;
  constexpr unsigned long kHalfPeriodMs = kPulsePeriodMs / 2;
  const unsigned long phase = now % kPulsePeriodMs;
  const unsigned long ramp = phase <= kHalfPeriodMs
                                 ? phase
                                 : kPulsePeriodMs - phase;
  return static_cast<uint8_t>(minimum +
      (static_cast<unsigned long>(maximum - minimum) * ramp) /
          kHalfPeriodMs);
}

uint32_t networkColor(unsigned long now) {
  if (networkLedState == NetworkLedState::AP_MODE) {
    const uint8_t level = pulseLevel(now, 4, 24);
    return keyLeds.Color(level, 0, level);
  }
  if (networkLedState == NetworkLedState::CONNECTING) {
    return keyLeds.Color(0, 0, pulseLevel(now, 3, 24));
  }
  return keyLeds.Color(0, 0, 16);
}

uint32_t pressedColor() {
  return keyLeds.Color(64, 16, 0);
}

void renderLeds(unsigned long now) {
  const uint32_t background = networkColor(now);
  for (size_t index = 0; index < LED_COUNT; ++index) {
    keyLeds.setPixelColor(KEY_LED_INDEX[index],
                          keys[index].stablePressed ? pressedColor()
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

  for (size_t index = 0; index < LED_COUNT; ++index) {
    const bool pressed = digitalRead(keys[index].pin) == LOW;
    keys[index].rawPressed = pressed;
    keys[index].stablePressed = pressed;
    keys[index].changedAtMs = millis();
  }
  renderLeds(millis());

  Serial.println("[ChainOSCmini][GPIO] KEY1=GPIO0 KEY2=GPIO17 debounce=20ms");
  Serial.println("[ChainOSCmini][GPIO] WS2812=GPIO21 PWR_EN=GPIO40 count=2");
  Serial.println("[ChainOSCmini][GPIO] ap=PURPLE_PULSE connecting=BLUE_PULSE connected=BLUE pressed=ORANGE");
}

void dualKeyHardwareUpdate() {
  const unsigned long now = millis();
  updateKey(0, now);
  updateKey(1, now);
  if (networkLedState != NetworkLedState::CONNECTED &&
      now - lastAnimationMs >= 50) {
    lastAnimationMs = now;
    renderLeds(now);
  }
}

void dualKeySetNetworkLedState(NetworkLedState state) {
  if (networkLedState == state) return;
  networkLedState = state;
  if (ledsReady) renderLeds(millis());
}
