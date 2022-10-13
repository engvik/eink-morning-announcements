#include <Arduino.h>

#include "eink_display.h"


void setup()
{
    init();
    refreshScreen();
    hibernate();
}

void loop() {};
