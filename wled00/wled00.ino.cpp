# 1 "C:\\Users\\axela\\AppData\\Local\\Temp\\tmp6cglndax"
#include <Arduino.h>
# 1 "D:/dev/fibra/02_WLed/wled00/wled00.ino"
# 13 "D:/dev/fibra/02_WLed/wled00/wled00.ino"
#include "wled.h"
void setup();
void loop();
#line 15 "D:/dev/fibra/02_WLed/wled00/wled00.ino"
void setup() {
  WLED::instance().setup();
}

void loop() {
  WLED::instance().loop();
}