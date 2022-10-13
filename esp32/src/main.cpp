#include <Arduino.h>

#include "eink_display.h"


void setup()
{
    EinkDisplay ed;
    ed.init();
    ed.refreshScreen();
    ed.hibernate();
}

void loop() {};
