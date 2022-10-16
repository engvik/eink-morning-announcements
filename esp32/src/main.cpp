#include <Arduino.h>

#include "eink_display.h"
#include "wifi.h"

void setup()
{
    // Init serial
    Serial.begin(115200);

    // Init WiFi
    initWiFi();

    // Init display
    Serial.println("Setting up Eink Display ..");
    
    EinkDisplay ed;
    ed.init();
    ed.refreshScreen();
    ed.hibernate();
}

void loop() {};
