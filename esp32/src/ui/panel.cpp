#include "ui/panel.h"

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

void drawUpper(Adafruit_GFX& gfx, const TextStyle& style, int16_t x,
               int16_t baseline, const char* text) {
  char upper[32];
  size_t i = 0;

  for (; text[i] != '\0' && i < sizeof(upper) - 1; i++) {
    const char c = text[i];
    upper[i] = (c >= 'a' && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c;
  }

  upper[i] = '\0';

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

}  // namespace ui
