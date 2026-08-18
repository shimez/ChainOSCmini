#pragma once

#include <stdint.h>

static constexpr const char* APP_NAME = "ChainOSCmini";
static constexpr const char* APP_VERSION = "0.2.0";
static constexpr unsigned long SERIAL_BAUD = 115200;
static constexpr unsigned long SERIAL_WAIT_MS = 2000;
static constexpr unsigned long BOOT_DIAGNOSTICS_DELAY_MS = 5000;
static constexpr unsigned long HEARTBEAT_INTERVAL_MS = 5000;

static constexpr bool HARDWARE_GPIO_ENABLED = true;

// Chain DualKey GPIO assignments confirmed against the product PinMap.
static constexpr uint8_t KEY1_PIN = 0;
static constexpr uint8_t KEY2_PIN = 17;
static constexpr uint8_t LED_DATA_PIN = 21;
static constexpr uint8_t LED_POWER_PIN = 40;
static constexpr uint8_t LED_COUNT = 2;
static constexpr unsigned long KEY_DEBOUNCE_MS = 20;

// SWITCH_1 (GPIO8) and SWITCH_2 (GPIO7) are intentionally not configured.
// Driving either pin HIGH can prevent the device from powering off normally.
