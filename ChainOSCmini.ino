/*
 * Arduino IDE entry point for ChainOSCmini.
 *
 * The shared implementation lives in src/app.cpp. PlatformIO does not build
 * this root sketch and provides setup()/loop() from src/main.cpp instead.
 */

#include "src/app.h"

void setup() {
  appSetup();
}

void loop() {
  appLoop();
}
