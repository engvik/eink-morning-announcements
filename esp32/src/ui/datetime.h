#pragma once

#include <cstddef>
#include <cstdint>

// Dates as the backend sends them. Timestamps arrive as RFC3339 already
// converted to the display's timezone, so the offset carries no information the
// panel needs and is not parsed.
namespace ui {

struct DateTime {
  int16_t year;
  int8_t month;
  int8_t day;
  int8_t hour;
  int8_t minute;
  bool valid;
};

// Reads 2026-09-05T06:21:00+02:00 and the date-only 2026-09-05. Anything that
// does not match returns valid == false rather than partial garbage.
DateTime parseIso8601(const char* text);

// Whole days from a to b, ignoring the time of day. Negative when b is earlier.
int16_t daysBetween(const DateTime& a, const DateTime& b);

bool sameDay(const DateTime& a, const DateTime& b);

// Writes HH:MM. Needs a capacity of 6; writes an empty string if smaller.
void formatClock(char* out, size_t capacity, const DateTime& time);

// Three letter upper case weekday, for the AHEAD column. Empty if invalid.
const char* weekdayAbbrev(const DateTime& date);

// First letter of the weekday, offset whole days from the given date. Used for
// the RUNNING strip's column headings.
char weekdayLetter(const DateTime& date, int16_t offset);

}  // namespace ui
