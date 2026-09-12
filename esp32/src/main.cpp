#include <Arduino.h>

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
  Serial.begin(SERIAL_BAUD);

  Serial.println("Setting up WiFi ..");

  if (!initWiFi()) {
    Serial.println("Unable to connect to WiFi, sleeping ..");
    deepSleep(SLEEP_TIME);
  }

  BackendClient backend;
  ui::DisplayModel model;

  if (!fetchDisplayData(backend, model)) {
    // Show stale data over broken data.
    Serial.println("No meta, leaving the panel as it is ..");
    deepSleep(SLEEP_TIME);
  }

  Serial.println("Drawing ..");

  renderDisplay(model);

  // One long sleep overnight, hourly the rest of the day. Without meta there
  // is no clock to decide on, so assume the short interval.
  const bool overnight = model.now.valid && model.now.hour == LONG_SLEEP_HOUR;

  deepSleep(overnight ? LONG_SLEEP_TIME : SLEEP_TIME);
}

void loop() {}
