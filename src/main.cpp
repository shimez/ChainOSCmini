#include "app.h"

// Arduino IDE builds ChainOSCmini.ino, while PlatformIO builds src/main.cpp.
// This guard prevents duplicate Arduino entry points.
#ifdef CHAINOSCMINI_PLATFORMIO
void setup() {
  appSetup();
}

void loop() {
  appLoop();
}
#endif
