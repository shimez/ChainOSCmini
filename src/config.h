#pragma once

#include <stdint.h>

static constexpr const char* APP_NAME = "ChainOSCmini";
static constexpr const char* APP_VERSION = "0.4.0";
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

// First-stage Chain test: only the GPIO5/GPIO6 connector is enabled.
static constexpr uint8_t CHAIN_TEST_RX_PIN = 5;
static constexpr uint8_t CHAIN_TEST_TX_PIN = 6;
static constexpr uint32_t CHAIN_BAUD = 115200;
static constexpr unsigned long CHAIN_SCAN_INTERVAL_MS = 2000;
static constexpr uint16_t CHAIN_MAX_DEVICES = 16;
static constexpr unsigned long CHAIN_KEY_POLL_INTERVAL_MS = 25;
static constexpr uint8_t CHAIN_KEY_LED_BRIGHTNESS = 60;

// SWITCH_1 (GPIO8) and SWITCH_2 (GPIO7) are intentionally not configured.
// Driving either pin HIGH can prevent the device from powering off normally.
