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

uint32_t idleColor() {
  return keyLeds.Color(0, 0, 16);
}

uint32_t pressedColor() {
  return keyLeds.Color(64, 16, 0);
}

void updateLed(size_t index, bool pressed) {
  keyLeds.setPixelColor(KEY_LED_INDEX[index],
                        pressed ? pressedColor() : idleColor());
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
  updateLed(index, key.stablePressed);
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

  for (size_t index = 0; index < LED_COUNT; ++index) {
    const bool pressed = digitalRead(keys[index].pin) == LOW;
    keys[index].rawPressed = pressed;
    keys[index].stablePressed = pressed;
    keys[index].changedAtMs = millis();
    keyLeds.setPixelColor(KEY_LED_INDEX[index],
                          pressed ? pressedColor() : idleColor());
  }
  keyLeds.show();

  Serial.println("[ChainOSCmini][GPIO] KEY1=GPIO0 KEY2=GPIO17 debounce=20ms");
  Serial.println("[ChainOSCmini][GPIO] WS2812=GPIO21 PWR_EN=GPIO40 count=2");
  Serial.println("[ChainOSCmini][GPIO] idle=BLUE pressed=ORANGE");
}

void dualKeyHardwareUpdate() {
  const unsigned long now = millis();
  updateKey(0, now);
  updateKey(1, now);
}
