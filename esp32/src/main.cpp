#include <Arduino.h>
#include <Arduino_JSON.h>

#include "battery.h"
#include "eink_display.h"
#include "http.h"
#include "wifi.h"

RTC_DATA_ATTR int wake_mode = 1;

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
    data.battery = getBatteryVoltage();

    // Init display
    Serial.println("Setting up Eink Display ..");
    
    EinkDisplay ed;
    ed.init();
    Serial.println("Refreshing Eink Display ..");
    ed.refreshScreen();
    ed.draw(&data);
    ed.off();

    // Deep sleep

    if (wake_mode == 1) {
        wake_mode = 0;
        esp_sleep_enable_timer_wakeup(SLEEP_UNTIL_MORNING * 1000000);
    } else {
        wake_mode = 1;
        esp_sleep_enable_timer_wakeup(SLEEP_UNTIL_HOME * 1000000);
    }

    Serial.flush();
    esp_deep_sleep_start();
}

void loop() {};
