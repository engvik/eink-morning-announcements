#include "ui/panel.h"

#include <cmath>
#include <cstdio>

#include "ui/text.h"
#include "ui/theme.h"

namespace ui {
namespace {

// Baselines within the header, measured from the top of the band. The design
// stacks a small weekday over large numerals on the left, and three lines of
// meta on the right.
constexpr int16_t HEADER_WEEKDAY_BASELINE = 14;
constexpr int16_t HEADER_NUMERAL_BASELINE = 58;
constexpr int16_t HEADER_META_BASELINE = 13;
constexpr int16_t HEADER_META_STEP = 15;

// Gap between the date numerals and the month beside them.
constexpr int16_t HEADER_MONTH_GAP = 8;

// The reminder bar's inner padding, and the gap between its label and message.
constexpr int16_t REMINDER_PADDING = 12;
constexpr int16_t REMINDER_LABEL_GAP = 12;

// Text sits on one baseline, optically centred in the 32px bar.
constexpr int16_t REMINDER_BASELINE = 21;

// Weather. The left block is the hero temperature over its summary line; the
// right block is the icon and condition over three meta tokens.
constexpr int16_t WEATHER_HERO_BASELINE = 55;
constexpr int16_t WEATHER_DEGREE_BASELINE = 30;
constexpr int16_t WEATHER_SUMMARY_BASELINE = 71;
constexpr int16_t WEATHER_BLOCK_GAP = 16;

constexpr int16_t WEATHER_ICON_TOP = 8;
constexpr int16_t WEATHER_CONDITION_BASELINE = 36;
constexpr int16_t WEATHER_RULE_OFFSET = 60;
constexpr int16_t WEATHER_TOKEN_BASELINE = 77;
constexpr int16_t WEATHER_ICON_GAP = 12;

// The least space that must remain between the three meta tokens.
constexpr int16_t TOKEN_GAP = 24;

// The hourly strip. Everything is measured from the top of the band, which
// begins with six pixels of padding before the first row of the columns.
constexpr int16_t HOURLY_PADDING = 6;
constexpr int16_t HOURLY_HOUR_BASELINE = 18;
constexpr int16_t HOURLY_ICON_TOP = 27;
constexpr int16_t HOURLY_TEMP_BASELINE = 73;
constexpr int16_t HOURLY_PRECIP_BASELINE = 94;
constexpr int16_t HOURLY_PROBABILITY_BASELINE = 118;

// The design capitalises weekday, month and condition. These come from Go's
// time package and MET's legend, both ASCII, so a byte-wise fold is enough.
// Pass a width to truncate rather than overrun.
void drawUpper(Adafruit_GFX& gfx, const TextStyle& style, int16_t x,
               int16_t baseline, const char* text, int16_t width = 0) {
  char upper[40];
  size_t i = 0;

  for (; text[i] != '\0' && i < sizeof(upper) - 1; i++) {
    const char c = text[i];
    upper[i] = (c >= 'a' && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c;
  }

  upper[i] = '\0';

  if (width > 0) {
    drawTruncated(gfx, style, x, baseline, width, upper);
    return;
  }

  drawLeft(gfx, style, x, baseline, upper);
}

}  // namespace

int16_t drawHeader(Adafruit_GFX& gfx, const DisplayModel& model, int16_t top) {
  const int16_t right = CONTENT_X + CONTENT_WIDTH;

  drawUpper(gfx, STYLE_LABEL, CONTENT_X, top + HEADER_WEEKDAY_BASELINE,
            model.header.weekday);

  // The numerals and the month share a baseline, as they do in the design.
  char date[4];
  snprintf(date, sizeof(date), "%u",
           static_cast<unsigned>(model.header.date) % 100);

  const int16_t baseline = top + HEADER_NUMERAL_BASELINE;

  drawLeft(gfx, STYLE_HERO, CONTENT_X, baseline, date);

  const int16_t monthX =
      CONTENT_X + measure(STYLE_HERO, date) + HEADER_MONTH_GAP;

  drawUpper(gfx, STYLE_TITLE, monthX, baseline, model.header.month);

  // Sunrise, sunset and week number, right aligned in a column.
  char line[16];
  int16_t metaBaseline = top + HEADER_META_BASELINE;

  if (model.header.sunrise[0] != '\0') {
    snprintf(line, sizeof(line), "RISE %s", model.header.sunrise);
    drawRight(gfx, STYLE_META, right, metaBaseline, line);
  }

  metaBaseline += HEADER_META_STEP;

  if (model.header.sunset[0] != '\0') {
    snprintf(line, sizeof(line), "SET %s", model.header.sunset);
    drawRight(gfx, STYLE_META, right, metaBaseline, line);
  }

  metaBaseline += HEADER_META_STEP;

  snprintf(line, sizeof(line), "WEEK %u",
           static_cast<unsigned>(model.header.week) % 100);
  drawRight(gfx, STYLE_META, right, metaBaseline, line);

  // The rule is the band's bottom edge, inside its height.
  gfx.fillRect(CONTENT_X, top + HEADER_HEIGHT - RULE_THICK, CONTENT_WIDTH,
               RULE_THICK, INK);

  return top + HEADER_HEIGHT;
}

int16_t drawReminder(Adafruit_GFX& gfx, const DisplayModel& model,
                     int16_t top) {
  // No message collapses the band entirely, gap included.
  if (model.reminder[0] == '\0') {
    return top;
  }

  const int16_t bandTop = top + BAND_GAP;

  gfx.fillRect(CONTENT_X, bandTop, CONTENT_WIDTH, REMINDER_HEIGHT, INK);

  const int16_t baseline = bandTop + REMINDER_BASELINE;
  const int16_t labelX = CONTENT_X + REMINDER_PADDING;

  drawLeft(gfx, STYLE_LABEL, labelX, baseline, "REMINDER", PAPER);

  const int16_t messageX =
      labelX + measure(STYLE_LABEL, "REMINDER") + REMINDER_LABEL_GAP;

  // Whatever is left of the bar, less the padding on the far side.
  const int16_t available =
      CONTENT_X + CONTENT_WIDTH - REMINDER_PADDING - messageX;

  drawTruncated(gfx, STYLE_TITLE, messageX, baseline, available, model.reminder,
                PAPER);

  return bandTop + REMINDER_HEIGHT;
}

int16_t drawWeather(Adafruit_GFX& gfx, const DisplayModel& model, int16_t top) {
  const int16_t bandTop = top + BAND_GAP;

  // The band keeps its height even with nothing to show, so a failed weather
  // fetch does not shift everything below it.
  if (!model.weather.valid) {
    return bandTop + WEATHER_HEIGHT;
  }

  const int16_t right = CONTENT_X + CONTENT_WIDTH;

  char temperature[8];
  snprintf(temperature, sizeof(temperature), "%d", model.weather.temperature);

  char low[16] = "";
  char feels[16] = "";

  if (model.weather.low != MISSING) {
    snprintf(low, sizeof(low), "LOW %d\xC2\xB0", model.weather.low);
  }

  if (model.weather.feels != MISSING) {
    snprintf(feels, sizeof(feels), "FEELS %d\xC2\xB0", model.weather.feels);
  }

  char summary[40];
  snprintf(summary, sizeof(summary), "%s%s%s", low,
           low[0] != '\0' && feels[0] != '\0' ? " \xC2\xB7 " : "", feels);

  drawLeft(gfx, STYLE_HERO, CONTENT_X, bandTop + WEATHER_HERO_BASELINE,
           temperature);

  // The degree sits at the top of the numerals rather than on their baseline.
  const int16_t degreeX = CONTENT_X + measure(STYLE_HERO, temperature);

  drawLeft(gfx, STYLE_TITLE, degreeX, bandTop + WEATHER_DEGREE_BASELINE,
           "\xC2\xB0");
  drawLeft(gfx, STYLE_META_WIDE, CONTENT_X, bandTop + WEATHER_SUMMARY_BASELINE,
           summary);

  // The summary line is usually wider than the numerals, so the right block
  // starts from whichever is wider rather than a fixed column.
  const int16_t heroWidth =
      measure(STYLE_HERO, temperature) + measure(STYLE_TITLE, "\xC2\xB0");
  const int16_t summaryWidth = measure(STYLE_META_WIDE, summary);
  const int16_t leftWidth = heroWidth > summaryWidth ? heroWidth : summaryWidth;

  const int16_t blockX = CONTENT_X + leftWidth + WEATHER_BLOCK_GAP;

  if (model.weather.icon != nullptr) {
    gfx.drawBitmap(blockX, bandTop + WEATHER_ICON_TOP, model.weather.icon,
                   HERO_ICON_SIZE, HERO_ICON_SIZE, INK);
  }

  const int16_t conditionX = blockX + HERO_ICON_SIZE + WEATHER_ICON_GAP;

  if (model.weather.condition != nullptr) {
    drawUpper(gfx, STYLE_TITLE, conditionX,
              bandTop + WEATHER_CONDITION_BASELINE, model.weather.condition,
              right - conditionX);
  }

  gfx.fillRect(blockX, bandTop + WEATHER_RULE_OFFSET, right - blockX, RULE_THIN,
               INK);

  // Three tokens spread across the block: outer two anchored, middle centred.
  // Gust in brackets when known.
  char speed[16] = "";

  if (model.weather.wind != MISSING && model.weather.gust != MISSING) {
    snprintf(speed, sizeof(speed), "%d (%d)", model.weather.wind,
             model.weather.gust);
  } else if (model.weather.wind != MISSING) {
    snprintf(speed, sizeof(speed), "%d", model.weather.wind);
  }

  char wind[28] = "";

  if (speed[0] != '\0') {
    snprintf(wind, sizeof(wind), "WIND %s M/S", speed);
  }

  char uv[12] = "";

  if (model.weather.uv != MISSING) {
    snprintf(uv, sizeof(uv), "UV %d", model.weather.uv);
  }

  char precipitation[12] = "";

  if (!std::isnan(model.weather.precipitation)) {
    snprintf(precipitation, sizeof(precipitation), "%.1f MM",
             static_cast<double>(model.weather.precipitation));
  }

  const int16_t tokenBaseline = bandTop + WEATHER_TOKEN_BASELINE;

  const int16_t uvWidth = measure(STYLE_META_WIDE, uv);
  const int16_t precipitationWidth = measure(STYLE_META_WIDE, precipitation);

  // Below freezing the summary line grows and squeezes this row, so the unit
  // goes first. The design drops units from the hourly strip on the same
  // reasoning, that mm and m/s are learned rather than read.
  int16_t windWidth = measure(STYLE_META_WIDE, wind);

  if (speed[0] != '\0' &&
      right - blockX - windWidth - uvWidth - precipitationWidth < TOKEN_GAP) {
    snprintf(wind, sizeof(wind), "WIND %s", speed);
    windWidth = measure(STYLE_META_WIDE, wind);
  }

  // space-between: the outer tokens anchor to the edges and the middle one
  // sits centred in what is left, rather than centred in the whole block,
  // which would let it collide with a wide wind reading.
  const int16_t slack =
      right - blockX - windWidth - uvWidth - precipitationWidth;

  drawLeft(gfx, STYLE_META_WIDE, blockX, tokenBaseline, wind);
  drawLeft(gfx, STYLE_META_WIDE, blockX + windWidth + slack / 2, tokenBaseline,
           uv);
  drawRight(gfx, STYLE_META_WIDE, right, tokenBaseline, precipitation);

  return bandTop + WEATHER_HEIGHT;
}

int16_t drawHourly(Adafruit_GFX& gfx, const DisplayModel& model, int16_t top) {
  const int16_t bandTop = top + BAND_GAP;

  for (uint8_t column = 0; column < model.hourCount; column++) {
    const HourModel& hour = model.hours[column];

    const int16_t left = CONTENT_X + column * HOUR_COLUMN_WIDTH;
    const int16_t centre = left + HOUR_COLUMN_WIDTH / 2;

    // Dividers sit between columns, never at the outer edges, and there is no
    // rule above or below: the design lets the icons carry the row.
    if (column > 0) {
      gfx.fillRect(left, bandTop + HOURLY_PADDING, RULE_THIN,
                   HOURLY_HEIGHT - HOURLY_PADDING, INK);
    }

    drawCentred(gfx, STYLE_META_WIDE, centre, bandTop + HOURLY_HOUR_BASELINE,
                hour.label);

    if (hour.icon != nullptr) {
      gfx.drawBitmap(centre - HOUR_ICON_SIZE / 2, bandTop + HOURLY_ICON_TOP,
                     hour.icon, HOUR_ICON_SIZE, HOUR_ICON_SIZE, INK);
    }

    if (hour.temperature != MISSING) {
      char temperature[12];
      snprintf(temperature, sizeof(temperature), "%d\xC2\xB0",
               hour.temperature);
      drawCentred(gfx, STYLE_TITLE, centre, bandTop + HOURLY_TEMP_BASELINE,
                  temperature);
    }

    // A dry hour is a hyphen rather than a zero, so the eye skips it.
    if (!std::isnan(hour.precipitation)) {
      char precipitation[8];

      if (hour.precipitation > 0.0f) {
        snprintf(precipitation, sizeof(precipitation), "%.1f",
                 static_cast<double>(hour.precipitation));
      } else {
        snprintf(precipitation, sizeof(precipitation), "-");
      }

      drawCentred(gfx, STYLE_META, centre, bandTop + HOURLY_PRECIP_BASELINE,
                  precipitation);
    }

    // Probability is already suppressed below the floor during decoding, so a
    // zero here means the slot stays empty.
    if (hour.probability > 0) {
      char probability[8];
      snprintf(probability, sizeof(probability), "%d%%", hour.probability);
      drawCentred(gfx, STYLE_META_WIDE, centre,
                  bandTop + HOURLY_PROBABILITY_BASELINE, probability);
    }
  }

  return bandTop + HOURLY_HEIGHT;
}

}  // namespace ui
