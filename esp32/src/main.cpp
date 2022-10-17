#include <Arduino.h>

#include "eink_display.h"
#include "http.h"
#include "wifi.h"

void setup()
{
    // Init serial
    Serial.begin(115200);

    // Init WiFi
    Serial.println("Setting up WiFi ..");
    initWiFi();

    Serial.println("Fetching calendar data ..");
    // Fetch data from backend
    String calendar = httpGET(BACKEND_CALENDAR_ENDPOINT);
    Serial.println(calendar);

    Serial.println("Fetching message ..");
    String message = httpGET(BACKEND_MESSAGE_ENDPOINT);
    Serial.println(message);

    Serial.println("Fetching weather data ..");
    String weather = httpGET(BACKEND_WEATHER_ENDPOINT);
    Serial.println(weather);

    // Init display
    Serial.println("Setting up Eink Display ..");
    
    EinkDisplay ed;
    ed.init();
    ed.refreshScreen();
    ed.hibernate();
}

void loop() {};
