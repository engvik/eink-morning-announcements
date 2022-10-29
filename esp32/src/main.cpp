#include <Arduino.h>
#include <Arduino_JSON.h>

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

    // Fetch data from backend
    Serial.println("Fetching calendar data ..");
    String rawCalendar = httpGET(BACKEND_CALENDAR_ENDPOINT);

    Serial.println("Fetching message ..");
    String rawMessage = httpGET(BACKEND_MESSAGE_ENDPOINT);

    Serial.println("Fetching meta data ..");
    String rawMeta = httpGET(BACKEND_META_ENDPOINT);

    Serial.println("Fetching weather data ..");
    String rawWeather = httpGET(BACKEND_WEATHER_ENDPOINT);

    struct DisplayData data;
    data.calendar = JSON.parse(rawCalendar);
    data.message = JSON.parse(rawMessage);
    data.meta = JSON.parse(rawMeta);
    data.weather = JSON.parse(rawWeather);

    // Init display
    Serial.println("Setting up Eink Display ..");
    
    EinkDisplay ed;
    ed.init();
    delay(5000);
    Serial.println("Refreshing Eink Display ..");
    ed.refreshScreen();
    delay(5000);
    ed.draw(&data);
    ed.off();
}

void loop() {};
