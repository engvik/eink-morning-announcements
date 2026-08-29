#include <Arduino.h>
#include <Arduino_JSON.h>

#include "battery.h"
#include "eink_display.h"
#include "http.h"
#include "wifi.h"

static void deepSleep(std::uint64_t seconds)
{
    esp_sleep_enable_timer_wakeup(seconds * uS_TO_S_FACTOR);
    Serial.flush();
    esp_deep_sleep_start();
}

void setup()
{
    // Init serial
    Serial.begin(115200);

    // Init WiFi
    Serial.println("Setting up WiFi ..");

    if (!initWiFi()) {
        Serial.println("Unable to connect to WiFi, sleeping ..");
        deepSleep(SLEEP_TIME);
    }

    // Fetch data from backend
    BackendClient backend;

    Serial.println("Fetching calendar data ..");
    HttpResponse rawCalendar = backend.get(BACKEND_CALENDAR_ENDPOINT);

    Serial.println("Fetching message ..");
    HttpResponse rawMessage = backend.get(BACKEND_MESSAGE_ENDPOINT);

    Serial.println("Fetching meta data ..");
    HttpResponse rawMeta = backend.get(BACKEND_META_ENDPOINT);

    Serial.println("Fetching weather data ..");
    HttpResponse rawWeather = backend.get(BACKEND_WEATHER_ENDPOINT);

    struct DisplayData data;
    data.calendar = JSON.parse(rawCalendar.body);
    data.message = JSON.parse(rawMessage.body);
    data.meta = JSON.parse(rawMeta.body);
    data.weather = JSON.parse(rawWeather.body);
    data.battery = getBatteryVoltage();

    // Init display
    Serial.println("Setting up Eink Display ..");
    
    EinkDisplay ed;
    ed.init();
    Serial.println("Refreshing Eink Display ..");
    ed.refreshScreen();
    ed.draw(&data);
    ed.hibernate();

    // Deep sleep
    const char* now = data.meta["now"];
    int hour = String(now).substring(11, 13).toInt();

    // Sleep for six times SLEEP_TIME at midnight, otherwise update every
    // SLEEP_TIME.
    if (hour == LONG_SLEEP_HOUR) {
        deepSleep(LONG_SLEEP_TIME);
    } else {
        deepSleep(SLEEP_TIME);
    }
}

void loop() {};
