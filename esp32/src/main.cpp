#include <Arduino.h>
#include <Arduino_JSON.h>

#include "data.h"
#include "eink_display.h"
#include "http.h"
#include "wifi.h"

[[noreturn]] static void deepSleep(std::uint64_t seconds) {
  esp_sleep_enable_timer_wakeup(seconds * uS_TO_S_FACTOR);
  Serial.flush();
  esp_deep_sleep_start();
}

void setup() {
  // Init serial
  Serial.begin(SERIAL_BAUD);

  // Init WiFi
  Serial.println("Setting up WiFi ..");

  if (!initWiFi()) {
    Serial.println("Unable to connect to WiFi, sleeping ..");
    deepSleep(SLEEP_TIME);
  }

  // Fetch data from backend
  BackendClient backend;
  DisplayData data = fetchDisplayData(backend);

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

void loop() {}
