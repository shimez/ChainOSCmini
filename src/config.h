#pragma once

static constexpr const char* APP_NAME = "ChainOSCmini";
static constexpr const char* APP_VERSION = "0.1.0";
static constexpr unsigned long SERIAL_BAUD = 115200;
static constexpr unsigned long SERIAL_WAIT_MS = 2000;
static constexpr unsigned long HEARTBEAT_INTERVAL_MS = 5000;

// Keep hardware-specific GPIO disabled until the production Chain DualKey
// pinout has been verified on the actual device.
static constexpr bool HARDWARE_GPIO_ENABLED = false;
