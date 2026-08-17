#include "diagnostics.h"

#include <Arduino.h>
#include <ESP.h>
#include <esp_system.h>

#include "config.h"

namespace {

const char* resetReasonText(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_POWERON: return "power-on";
    case ESP_RST_EXT: return "external";
    case ESP_RST_SW: return "software";
    case ESP_RST_PANIC: return "panic";
    case ESP_RST_INT_WDT: return "interrupt-watchdog";
    case ESP_RST_TASK_WDT: return "task-watchdog";
    case ESP_RST_WDT: return "other-watchdog";
    case ESP_RST_DEEPSLEEP: return "deep-sleep";
    case ESP_RST_BROWNOUT: return "brownout";
    case ESP_RST_SDIO: return "sdio";
    case ESP_RST_UNKNOWN:
    default: return "unknown";
  }
}

void printBytes(const char* label, uint32_t bytes) {
  Serial.printf("[ChainOSCmini][BOOT] %s=%lu bytes\n", label,
                static_cast<unsigned long>(bytes));
}

}  // namespace

void printBootDiagnostics() {
  Serial.println();
  Serial.println("========================================");
  Serial.printf("%s v%s\n", APP_NAME, APP_VERSION);
  Serial.println("Chain DualKey bring-up firmware");
  Serial.println("========================================");
  Serial.printf("[ChainOSCmini][BOOT] build=%s %s\n", __DATE__, __TIME__);
  Serial.printf("[ChainOSCmini][BOOT] chip=%s revision=%u cores=%u\n",
                ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores());
  Serial.printf("[ChainOSCmini][BOOT] cpu=%u MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("[ChainOSCmini][BOOT] reset=%s (%d)\n",
                resetReasonText(esp_reset_reason()),
                static_cast<int>(esp_reset_reason()));
  printBytes("flash", ESP.getFlashChipSize());
  printBytes("sketch", ESP.getSketchSize());
  printBytes("free_sketch", ESP.getFreeSketchSpace());
  printBytes("heap", ESP.getHeapSize());
  printBytes("free_heap", ESP.getFreeHeap());
  printBytes("psram", ESP.getPsramSize());
  printBytes("free_psram", ESP.getFreePsram());
  Serial.printf("[ChainOSCmini][BOOT] hardware_gpio=%s\n",
                HARDWARE_GPIO_ENABLED ? "enabled" : "disabled (safe mode)");
  Serial.println("[ChainOSCmini][BOOT] READY");
}

void printHeartbeat() {
  Serial.printf("[ChainOSCmini][RUN] uptime=%lu ms free_heap=%lu bytes\n",
                static_cast<unsigned long>(millis()),
                static_cast<unsigned long>(ESP.getFreeHeap()));
}
