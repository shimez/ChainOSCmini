#pragma once

#include <stdint.h>

static constexpr const char* APP_NAME = "ChainOSCmini";
static constexpr const char* APP_VERSION = "1.4.3";
static constexpr unsigned long SERIAL_BAUD = 115200;
static constexpr unsigned long BOOT_DIAGNOSTICS_DELAY_MS = 5000;
static constexpr unsigned long HEARTBEAT_INTERVAL_MS = 5000;

// Enable only while diagnosing Web UI response performance.
#define CHAINOSCMINI_WEB_PERF_DEBUG 0
// Report per-device file size and LittleFS total/used/free capacity.
#define CHAINOSCMINI_STORAGE_DEBUG 1

static constexpr bool HARDWARE_GPIO_ENABLED = true;

// Chain DualKey GPIO assignments confirmed against the product PinMap.
static constexpr uint8_t KEY1_PIN = 0;
static constexpr uint8_t KEY2_PIN = 17;
static constexpr uint8_t LED_DATA_PIN = 21;
static constexpr uint8_t LED_POWER_PIN = 40;
static constexpr uint8_t LED_COUNT = 2;
static constexpr unsigned long KEY_DEBOUNCE_MS = 20;

// Independent UART assignments for both Chain connectors.
static constexpr uint8_t CHAIN_G5_G6_RX_PIN = 5;
static constexpr uint8_t CHAIN_G5_G6_TX_PIN = 6;
static constexpr uint8_t CHAIN_G47_G48_RX_PIN = 47;
static constexpr uint8_t CHAIN_G47_G48_TX_PIN = 48;
static constexpr uint32_t CHAIN_BAUD = 115200;
static constexpr unsigned long CHAIN_SCAN_INTERVAL_MS = 2000;
static constexpr uint16_t CHAIN_MAX_DEVICES = 16;
static constexpr unsigned long CHAIN_KEY_POLL_INTERVAL_MS = 25;
static constexpr uint8_t CHAIN_KEY_LED_BRIGHTNESS = 60;

// Wi-Fi provisioning and local web access.
static constexpr const char* WIFI_AP_SSID = "ChainOSCmini-Setup";
static constexpr const char* WIFI_AP_PASSWORD = "12345678";
static constexpr const char* WIFI_MDNS_HOST = "chainoscmini";
static constexpr const char* WIFI_PREFS_NAMESPACE = "chainoscmini";
static constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000;
static constexpr unsigned long NETWORK_RESTART_DELAY_MS = 1200;
static constexpr uint8_t CAPTIVE_DNS_PORT = 53;
// ESP-IDF uses quarter-dBm units. 8 = 2 dBm. The Chain DualKey test unit
// resets during client association at the framework default TX power.
static constexpr int8_t WIFI_TX_POWER_QDBM = 8;

// SWITCH_1 (GPIO8) and SWITCH_2 (GPIO7) are intentionally not configured.
// Driving either pin HIGH can prevent the device from powering off normally.
