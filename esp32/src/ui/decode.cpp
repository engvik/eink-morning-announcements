#include "ui/decode.h"

#include <cmath>
#include <cstdio>
#include <cstring>

#include "cjson/cJSON.h"
#include "icons/symbols.h"
#include "ui/datetime.h"
#include "ui/theme.h"

namespace ui {
namespace {

// Every hourly column is this far apart, which is what the design's 08/10/12
// spacing means.
constexpr int HOUR_STEP = 2;

void copyString(char* out, size_t capacity, const char* value) {
  if (value == nullptr) {
    out[0] = '\0';
    return;
  }

  std::strncpy(out, value, capacity - 1);
  out[capacity - 1] = '\0';
}

const cJSON* field(const cJSON* object, const char* key) {
  return cJSON_GetObjectItemCaseSensitive(object, key);
}

const char* text(const cJSON* object, const char* key) {
  const cJSON* item = field(object, key);

  return cJSON_IsString(item) ? item->valuestring : nullptr;
}

double number(const cJSON* object, const char* key, double fallback = 0.0) {
  const cJSON* item = field(object, key);

  return cJSON_IsNumber(item) ? item->valuedouble : fallback;
}

int16_t rounded(double value) {
  return static_cast<int16_t>(std::lround(value));
}

int16_t whole(const cJSON* object, const char* key) {
  const cJSON* item = field(object, key);

  return cJSON_IsNumber(item) ? rounded(item->valuedouble) : MISSING;
}

float decimal(const cJSON* object, const char* key) {
  const cJSON* item = field(object, key);

  return cJSON_IsNumber(item) ? static_cast<float>(item->valuedouble) : NAN;
}

// Copies HH:MM out of an RFC3339 timestamp.
void copyClock(char* out, size_t capacity, const char* timestamp) {
  const DateTime parsed = parseIso8601(timestamp);

  if (!parsed.valid) {
    out[0] = '\0';
    return;
  }

  // parseIso8601 already rejects out of range values, but the compiler cannot
  // see that through int8_t and sizes the buffer for a negative sign.
  const int hour = parsed.hour & 0x1F;
  const int minute = parsed.minute & 0x3F;

  snprintf(out, capacity, "%02d:%02d", hour, minute);
}

}  // namespace

void clear(DisplayModel& model) { model = DisplayModel{}; }

void decodeMeta(DisplayModel& model, const char* json) {
  cJSON* root = cJSON_Parse(json);

  if (root == nullptr) {
    return;
  }

  copyString(model.header.weekday, sizeof(model.header.weekday),
             text(root, "today"));
  copyString(model.header.month, sizeof(model.header.month),
             text(root, "month"));

  model.header.date = rounded(number(root, "date"));
  model.header.week = rounded(number(root, "week"));

  copyClock(model.header.sunrise, sizeof(model.header.sunrise),
            text(root, "sunrise"));
  copyClock(model.header.sunset, sizeof(model.header.sunset),
            text(root, "sunset"));

  // The footer's clock is when the panel last woke, which is this timestamp.
  copyClock(model.updated, sizeof(model.updated), text(root, "now"));

  cJSON_Delete(root);
}

void decodeMessage(DisplayModel& model, const char* json) {
  cJSON* root = cJSON_Parse(json);

  if (root == nullptr) {
    // A 404 body is not JSON. No message means the band collapses, which is a
    // layout state rather than an error.
    return;
  }

  copyString(model.reminder, sizeof(model.reminder), text(root, "message"));

  cJSON_Delete(root);
}

void decodeWeather(DisplayModel& model, const char* json) {
  cJSON* root = cJSON_Parse(json);

  if (root == nullptr) {
    return;
  }

  const cJSON* forecasts = field(root, "forecasts");
  const cJSON* days = field(root, "days");

  const cJSON* first =
      cJSON_IsArray(forecasts) ? cJSON_GetArrayItem(forecasts, 0) : nullptr;

  if (first != nullptr) {
    const cJSON* instant = field(first, "instant");
    const cJSON* oneHour = field(first, "one_hour");

    model.weather.temperature = whole(instant, "air_temperature");
    model.weather.feels = whole(instant, "apparent_air_temperature");
    model.weather.wind = whole(instant, "wind_speed");
    model.weather.gust = whole(instant, "wind_speed_of_gust");

    const WeatherSymbol* symbol = findSymbol(text(oneHour, "symbol_code"));

    if (symbol != nullptr) {
      model.weather.icon = symbol->icon44;
      model.weather.condition = symbol->label;
    }

    // The hero is the temperature; without it there is no band.
    model.weather.valid = model.weather.temperature != MISSING;
  }

  // Today's aggregates come from the first day bucket.
  const cJSON* today =
      cJSON_IsArray(days) ? cJSON_GetArrayItem(days, 0) : nullptr;

  if (today != nullptr) {
    model.weather.low = whole(today, "air_temperature_min");
    model.weather.uv = whole(today, "ultraviolet_index_max");
    model.weather.precipitation = decimal(today, "precipitation_amount");
  }

  // The strip samples every other forecast, giving the design's two hour step.
  if (cJSON_IsArray(forecasts)) {
    const int available = cJSON_GetArraySize(forecasts);

    for (size_t column = 0; column < MAX_HOURS; column++) {
      const int index = static_cast<int>(column) * HOUR_STEP;

      if (index >= available) {
        break;
      }

      const cJSON* entry = cJSON_GetArrayItem(forecasts, index);
      const cJSON* instant = field(entry, "instant");
      const cJSON* oneHour = field(entry, "one_hour");

      HourModel& hour = model.hours[column];

      const DateTime at = parseIso8601(text(entry, "time"));
      if (at.valid) {
        snprintf(hour.label, sizeof(hour.label), "%02d", at.hour & 0x1F);
      }

      hour.temperature = whole(instant, "air_temperature");
      hour.precipitation = decimal(oneHour, "precipitation_amount");

      const int16_t probability =
          rounded(number(oneHour, "probability_of_precipitation"));

      // Below the floor the slot stays empty, so dry hours read as dry.
      hour.probability =
          probability >= static_cast<int16_t>(PROBABILITY_FLOOR) ? probability
                                                                 : 0;

      const WeatherSymbol* symbol = findSymbol(text(oneHour, "symbol_code"));
      if (symbol != nullptr) {
        hour.icon = symbol->icon24;
      }

      model.hourCount = static_cast<uint8_t>(column + 1);
    }
  }

  cJSON_Delete(root);
}

}  // namespace ui
