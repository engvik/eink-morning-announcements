#include "data.h"

#include <cstring>

#include "battery.h"
#include "config.h"
#include "ui/decode.h"

bool fetchDisplayData(BackendClient& backend, ui::DisplayModel& model) {
  ui::clear(model);

  Serial.println("Fetching meta data ..");

  {
    const HttpResponse meta = backend.get(BACKEND_META_ENDPOINT);

    if (meta.ok()) {
      ui::decodeMeta(model, meta.body.c_str());
    }
  }

  // Stop if unable to fetch meta, as thay provide the context the upcoming data
  // is for.
  if (!model.now.valid) {
    return false;
  }

  Serial.println("Fetching weather data ..");
  const HttpResponse weather = backend.get(BACKEND_WEATHER_ENDPOINT);

  Serial.println("Fetching message ..");
  const HttpResponse message = backend.get(BACKEND_MESSAGE_ENDPOINT);

  Serial.println("Fetching calendar data ..");
  const HttpResponse calendar = backend.get(BACKEND_CALENDAR_ENDPOINT);

  // Weather first, it fills the day buckets the calendar summarises.
  if (weather.ok()) {
    ui::decodeWeather(model, weather.body.c_str());
  }

  if (message.ok()) {
    ui::decodeMessage(model, message.body.c_str());
  }

  if (calendar.ok()) {
    ui::decodeCalendar(model, calendar.body.c_str());
  }

  std::strncpy(model.location, DISPLAY_LOCATION, sizeof(model.location) - 1);
  model.battery =
      static_cast<int16_t>(getBatteryPercentage(getBatteryVoltage()));

  return true;
}
