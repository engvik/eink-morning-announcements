#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>

#include "ui/datetime.h"

// What the panel draws, decoded from the backend once and owned outright.
//
// Everything is a fixed buffer rather than a pointer into the parsed JSON, so
// the draw layer has no dependency on the parser and nothing to outlive. The
// whole model is about 6 KB, too much for the loop task stack, so keep it
// static.
namespace ui {

// An absent reading, drawn as nothing. Floats use NAN.
constexpr int16_t MISSING = INT16_MIN;

constexpr size_t MAX_TITLE = 64;
constexpr size_t MAX_TODAY = 20;
constexpr size_t MAX_AHEAD = 16;
constexpr size_t MAX_RUNNING = 4;
constexpr size_t MAX_HOURS = 5;
constexpr size_t RUNNING_DAYS = 4;
constexpr size_t MAX_DAYS = 10;
constexpr size_t MAX_NEWS = 16;

// Headlines outrun MAX_TITLE, so they get room to clip on width instead.
constexpr size_t MAX_HEADLINE = 128;

struct HeaderModel {
  char weekday[16];
  char month[16];
  char sunrise[6];  // HH:MM, empty above the polar circles
  char sunset[6];
  int16_t date;
  int16_t week;
};

struct WeatherModel {
  const unsigned char* icon;  // 44x44, null when the symbol is unknown
  const char* condition;      // owned by the symbol table, not copied
  int16_t temperature = MISSING;
  int16_t low = MISSING;
  int16_t feels = MISSING;
  int16_t wind = MISSING;
  int16_t gust = MISSING;
  int16_t uv = MISSING;
  float precipitation = NAN;
  bool valid;
};

struct HourModel {
  const unsigned char* icon;  // 24x24
  char label[3];              // hour of day, zero padded
  int16_t temperature = MISSING;
  float precipitation = NAN;  // zero draws as a hyphen
  int16_t probability;        // below PROBABILITY_FLOOR draws as nothing
};

struct EventModel {
  char time[14];  // 09:00, or 09:00-09:30 when the end differs
  char title[MAX_TITLE];
};

struct AheadModel {
  char day[8];   // THU 27
  char time[6];  // 17:30
  char title[MAX_TITLE];
  char summary[28];  // 14 6.0 MM, already formatted. The degree and
                     // middot are two bytes each in UTF-8.
};

// A day's weather, used to summarise the AHEAD rows.
struct DayModel {
  DateTime date;  // as the backend buckets them
  int16_t high = MISSING;
  float precipitation = NAN;
};

struct RunningModel {
  char title[MAX_TITLE];
  bool days[RUNNING_DAYS];  // which of the coming days it covers
};

struct NewsModel {
  char source[16];  // upper case, as the design sets labels
  char title[MAX_HEADLINE];
};

struct DisplayModel {
  HeaderModel header;
  WeatherModel weather;
  HourModel hours[MAX_HOURS];
  DayModel days[MAX_DAYS];
  RunningModel running[MAX_RUNNING];
  EventModel today[MAX_TODAY];
  AheadModel ahead[MAX_AHEAD];
  NewsModel news[MAX_NEWS];

  char reminder[MAX_TITLE];  // empty collapses the band
  char location[16];
  char updated[6];  // HH:MM

  // Today, as the backend sees it. Decoded from meta and used to sort events
  // into today, ahead and running.
  DateTime now;

  uint8_t hourCount;
  uint8_t dayCount;
  uint8_t runningCount;
  uint8_t todayCount;
  uint8_t aheadCount;
  uint8_t newsCount;

  int16_t todayTotal;         // today's events, before the MAX_TODAY cap
  int16_t battery = MISSING;  // percent
};

}  // namespace ui
