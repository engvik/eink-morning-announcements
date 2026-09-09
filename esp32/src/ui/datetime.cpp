#include "ui/datetime.h"

#include <cstring>

namespace ui {
namespace {

bool digits(const char* text, int count, int16_t& out) {
  int16_t value = 0;

  for (int i = 0; i < count; i++) {
    if (text[i] < '0' || text[i] > '9') {
      return false;
    }

    value = static_cast<int16_t>(value * 10 + (text[i] - '0'));
  }

  out = value;

  return true;
}

// Days since 1970-01-01 for a proleptic Gregorian date. Shifting the year so
// it starts in March puts the leap day at the end of the cycle, which is what
// removes the special casing.
int32_t daysFromCivil(int16_t year, int8_t month, int8_t day) {
  int32_t y = year - (month <= 2 ? 1 : 0);
  const int32_t era = (y >= 0 ? y : y - 399) / 400;
  const uint32_t yoe = static_cast<uint32_t>(y - era * 400);
  const uint32_t doy =
      (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
  const uint32_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;

  return era * 146097 + static_cast<int32_t>(doe) - 719468;
}

}  // namespace

DateTime parseIso8601(const char* text) {
  DateTime out{};

  if (text == nullptr || std::strlen(text) < 10) {
    return out;
  }

  int16_t year = 0, month = 0, day = 0;

  if (!digits(text, 4, year) || text[4] != '-' || !digits(text + 5, 2, month) ||
      text[7] != '-' || !digits(text + 8, 2, day)) {
    return out;
  }

  if (month < 1 || month > 12 || day < 1 || day > 31) {
    return out;
  }

  out.year = year;
  out.month = static_cast<int8_t>(month);
  out.day = static_cast<int8_t>(day);

  // A bare date is valid; the time is optional.
  if (std::strlen(text) >= 16 && text[10] == 'T') {
    int16_t hour = 0, minute = 0;

    if (!digits(text + 11, 2, hour) || text[13] != ':' ||
        !digits(text + 14, 2, minute) || hour > 23 || minute > 59) {
      return out;
    }

    out.hour = static_cast<int8_t>(hour);
    out.minute = static_cast<int8_t>(minute);
  }

  out.valid = true;

  return out;
}

int16_t daysBetween(const DateTime& a, const DateTime& b) {
  if (!a.valid || !b.valid) {
    return 0;
  }

  return static_cast<int16_t>(daysFromCivil(b.year, b.month, b.day) -
                              daysFromCivil(a.year, a.month, a.day));
}

bool sameDay(const DateTime& a, const DateTime& b) {
  return a.valid && b.valid && a.year == b.year && a.month == b.month &&
         a.day == b.day;
}

const char* weekdayAbbrev(const DateTime& date) {
  if (!date.valid) {
    return "";
  }

  // 1970-01-01 was a Thursday.
  static const char* const NAMES[] = {"THU", "FRI", "SAT", "SUN",
                                      "MON", "TUE", "WED"};

  const int32_t days = daysFromCivil(date.year, date.month, date.day);

  return NAMES[((days % 7) + 7) % 7];
}

}  // namespace ui
