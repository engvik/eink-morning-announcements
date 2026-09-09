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
  const char* now = text(root, "now");

  copyClock(model.updated, sizeof(model.updated), now);
  model.now = parseIso8601(now);

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

  // Keep every day bucket; the AHEAD rows summarise the ones they land on.
  if (cJSON_IsArray(days)) {
    const int count = cJSON_GetArraySize(days);

    for (int i = 0; i < count && model.dayCount < MAX_DAYS; i++) {
      const cJSON* entry = cJSON_GetArrayItem(days, i);
      DayModel& day = model.days[model.dayCount];

      copyString(day.date, sizeof(day.date), text(entry, "date"));
      day.high = whole(entry, "air_temperature_max");
      day.precipitation = decimal(entry, "precipitation_amount");

      model.dayCount++;
    }
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

namespace {

// Appends " (location)" when there is one, the convention the design keeps.
void composeTitle(char* out, size_t capacity, const char* title,
                  const char* location) {
  if (location == nullptr || location[0] == '\0') {
    copyString(out, capacity, title);
    return;
  }

  snprintf(out, capacity, "%s (%s)", title == nullptr ? "" : title, location);
}

// 09:00, or 09:00-09:30 when the event has a distinct end on the same day.
void composeTime(char* out, size_t capacity, const DateTime& start,
                 const DateTime& end) {
  const int startHour = start.hour & 0x1F;
  const int startMinute = start.minute & 0x3F;

  const bool ranged = end.valid && sameDay(start, end) &&
                      (end.hour != start.hour || end.minute != start.minute);

  if (!ranged) {
    snprintf(out, capacity, "%02d:%02d", startHour, startMinute);
    return;
  }

  snprintf(out, capacity, "%02d:%02d-%02d:%02d", startHour, startMinute,
           end.hour & 0x1F, end.minute & 0x3F);
}

// Looks up a day bucket by date, for the AHEAD summary.
const DayModel* findDay(const DisplayModel& model, const char* date) {
  for (uint8_t i = 0; i < model.dayCount; i++) {
    if (std::strcmp(model.days[i].date, date) == 0) {
      return &model.days[i];
    }
  }

  return nullptr;
}

void composeSummary(char* out, size_t capacity, const DisplayModel& model,
                    const DateTime& start) {
  // Masked for the same reason as the clock fields: parseIso8601 has already
  // range checked these, but the compiler only sees int8_t.
  char date[11];
  snprintf(date, sizeof(date), "%04u-%02u-%02u",
           static_cast<unsigned>(start.year) % 10000,
           static_cast<unsigned>(start.month) % 100,
           static_cast<unsigned>(start.day) % 100);

  const DayModel* day = findDay(model, date);

  if (day == nullptr) {
    out[0] = '\0';
    return;
  }

  char high[10] = "";
  char rain[12] = "";

  if (day->high != MISSING) {
    snprintf(high, sizeof(high), "%d\xC2\xB0", day->high);
  }

  if (!std::isnan(day->precipitation)) {
    snprintf(rain, sizeof(rain), "%.1f MM",
             static_cast<double>(day->precipitation));
  }

  snprintf(out, capacity, "%s%s%s", high,
           high[0] != '\0' && rain[0] != '\0' ? " \xC2\xB7 " : "", rain);
}

}  // namespace

void decodeCalendar(DisplayModel& model, const char* json) {
  cJSON* root = cJSON_Parse(json);

  if (root == nullptr) {
    return;
  }

  model.todayTotal = rounded(number(root, "total"));

  const cJSON* events = field(root, "events");

  if (!cJSON_IsArray(events) || !model.now.valid) {
    cJSON_Delete(root);
    return;
  }

  const int count = cJSON_GetArraySize(events);

  // Events arrive sorted by start, so the first one seen for a day is its
  // earliest, which is what an AHEAD row shows.
  int16_t lastAheadDay = 0;

  for (int i = 0; i < count; i++) {
    const cJSON* entry = cJSON_GetArrayItem(events, i);

    const DateTime start = parseIso8601(text(entry, "start"));
    const DateTime end = parseIso8601(text(entry, "end"));

    if (!start.valid) {
      continue;
    }

    const bool allDay = cJSON_IsTrue(field(entry, "all_day"));
    const char* title = text(entry, "title");
    const char* location = text(entry, "location");

    // All-day events are context rather than appointments, so they run along
    // the top with a strip of the days they cover instead of taking a row.
    if (allDay) {
      if (model.runningCount >= MAX_RUNNING) {
        continue;
      }

      const int16_t from = daysBetween(model.now, start);
      // DTEND is exclusive for an all-day event, so this is one past the last
      // day it covers.
      const int16_t until = end.valid ? daysBetween(model.now, end)
                                      : static_cast<int16_t>(from + 1);

      RunningModel& running = model.running[model.runningCount];
      bool visible = false;

      for (size_t day = 0; day < RUNNING_DAYS; day++) {
        const int16_t offset = static_cast<int16_t>(day);

        running.days[day] = offset >= from && offset < until;
        visible = visible || running.days[day];
      }

      // Nothing inside the window means it is not worth a row yet.
      if (!visible) {
        continue;
      }

      composeTitle(running.title, sizeof(running.title), title, location);
      model.runningCount++;

      continue;
    }

    const int16_t offset = daysBetween(model.now, start);

    if (offset <= 0) {
      if (model.todayCount >= MAX_TODAY) {
        continue;
      }

      EventModel& event = model.today[model.todayCount];

      composeTime(event.time, sizeof(event.time), start, end);
      composeTitle(event.title, sizeof(event.title), title, location);

      model.todayCount++;

      continue;
    }

    // One AHEAD row per upcoming day, showing that day's first event.
    if (offset == lastAheadDay || model.aheadCount >= MAX_AHEAD) {
      continue;
    }

    lastAheadDay = offset;

    AheadModel& ahead = model.ahead[model.aheadCount];

    snprintf(ahead.day, sizeof(ahead.day), "%s %d", weekdayAbbrev(start),
             start.day);
    snprintf(ahead.time, sizeof(ahead.time), "%02d:%02d", start.hour & 0x1F,
             start.minute & 0x3F);

    composeTitle(ahead.title, sizeof(ahead.title), title, location);
    composeSummary(ahead.summary, sizeof(ahead.summary), model, start);

    model.aheadCount++;
  }

  cJSON_Delete(root);
}

}  // namespace ui
