#include "data.h"

#include "battery.h"
#include "config.h"

DisplayData fetchDisplayData(BackendClient& backend) {
  Serial.println("Fetching calendar data ..");
  HttpResponse rawCalendar = backend.get(BACKEND_CALENDAR_ENDPOINT);

  Serial.println("Fetching message ..");
  HttpResponse rawMessage = backend.get(BACKEND_MESSAGE_ENDPOINT);

  Serial.println("Fetching meta data ..");
  HttpResponse rawMeta = backend.get(BACKEND_META_ENDPOINT);

  Serial.println("Fetching weather data ..");
  HttpResponse rawWeather = backend.get(BACKEND_WEATHER_ENDPOINT);

  DisplayData data;

  if (rawCalendar.ok()) {
    data.calendar = JSON.parse(rawCalendar.body);
  }

  if (rawMessage.ok()) {
    data.message = JSON.parse(rawMessage.body);
  }

  if (rawMeta.ok()) {
    data.meta = JSON.parse(rawMeta.body);
  }

  if (rawWeather.ok()) {
    data.weather = JSON.parse(rawWeather.body);
  }

  data.battery = getBatteryVoltage();

  return data;
}
