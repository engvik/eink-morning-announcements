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

    Serial.println("Fetching calendar data ..");
    // Fetch data from backend
    String rawCalendar = httpGET(BACKEND_CALENDAR_ENDPOINT);
    JSONVar calendar = JSON.parse(rawCalendar);
    Serial.println(calendar.length());

    Serial.println("Fetching message ..");
    String rawMessage = httpGET(BACKEND_MESSAGE_ENDPOINT);
    JSONVar message = JSON.parse(rawMessage);
    Serial.println(message["message"]);

    Serial.println("Fetching weather data ..");
    String rawWeather = httpGET(BACKEND_WEATHER_ENDPOINT);
    JSONVar weather = JSON.parse(rawWeather);
    Serial.println(weather.length());

    // Init display
    Serial.println("Setting up Eink Display ..");
    
    EinkDisplay ed;
    ed.init();
    ed.refreshScreen();
    ed.hibernate();
}

void loop() {};
