#pragma once

#include <Adafruit_GFX.h>

#include "fonts/ArchivoBlack56.h"
#include "fonts/ArchivoBold19.h"
#include "fonts/ArchivoSemiBold16.h"
#include "fonts/MonoRegular13.h"
#include "fonts/MonoSemiBold15.h"

namespace ui {

// GxEPD2 and GFXcanvas1 disagree about what a colour value means: the panel
// takes GxEPD_BLACK, which is zero, while the canvas sets a bit for any
// non-zero value. Drawing code says ink or paper and stays portable.
#ifdef PREVIEW_HOST
constexpr uint16_t INK = 1;
constexpr uint16_t PAPER = 0;
#else
// GxEPD_BLACK and GxEPD_WHITE, checked in eink_display.cpp.
constexpr uint16_t INK = 0x0000;
constexpr uint16_t PAPER = 0xFFFF;
#endif

struct Rect {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;

  constexpr int16_t right() const { return x + w; }
  constexpr int16_t bottom() const { return y + h; }
};

// A face plus the extra pixels to insert between glyphs. The design specifies
// tracking in em; at these sizes it lands on whole pixels.
struct TextStyle {
  const GFXfont* font;
  int8_t tracking;
};

// The panel is 800x480, rotated to portrait.
constexpr int16_t PANEL_WIDTH = 480;
constexpr int16_t PANEL_HEIGHT = 800;

constexpr int16_t DEFAULT_PADDING = 20;

// Parses a DISPLAY_PADDING_* value: empty is the default, non-digits are -1.
constexpr int16_t parseInset(const char* value) {
  if (*value == '\0') return DEFAULT_PADDING;
  int16_t inset = 0;
  for (; *value != '\0'; value++) {
    if (*value < '0' || *value > '9') return -1;
    inset = inset * 10 + (*value - '0');
  }
  return inset;
}

// Per-edge insets, so the content clears the frame's mat.
constexpr int16_t PADDING_TOP = parseInset(CFG_DISPLAY_PADDING_TOP);
constexpr int16_t PADDING_RIGHT = parseInset(CFG_DISPLAY_PADDING_RIGHT);
constexpr int16_t PADDING_BOTTOM = parseInset(CFG_DISPLAY_PADDING_BOTTOM);
constexpr int16_t PADDING_LEFT = parseInset(CFG_DISPLAY_PADDING_LEFT);
static_assert(PADDING_TOP >= 0, "DISPLAY_PADDING_TOP must be a whole number");
static_assert(PADDING_RIGHT >= 0,
              "DISPLAY_PADDING_RIGHT must be a whole number");
static_assert(PADDING_BOTTOM >= 0,
              "DISPLAY_PADDING_BOTTOM must be a whole number");
static_assert(PADDING_LEFT >= 0, "DISPLAY_PADDING_LEFT must be a whole number");

constexpr int16_t CONTENT_X = PADDING_LEFT;
constexpr int16_t CONTENT_WIDTH =
    PANEL_WIDTH - PADDING_LEFT - PADDING_RIGHT;  // 440 by default

constexpr int16_t RULE_THIN = 1;
constexpr int16_t RULE_THICK = 3;

// Band heights. Borders sit inside these, as they do in the design.
constexpr int16_t HEADER_HEIGHT = 64;
constexpr int16_t REMINDER_HEIGHT = 32;
constexpr int16_t WEATHER_HEIGHT = 71;
constexpr int16_t HOURLY_HEIGHT = 118;
constexpr int16_t FOOTER_HEIGHT = 28;

// The footer is anchored to the bottom, so the agenda gets whatever is left
// between the band above it and this.
constexpr int16_t FOOTER_TOP =
    PANEL_HEIGHT - PADDING_BOTTOM - FOOTER_HEIGHT;  // 752 by default

constexpr int16_t BAND_GAP = 12;
constexpr int16_t AGENDA_GAP = 8;

// Everything below the reminder shifts up by this much when there is no
// message, which is what buys the agenda two more rows.
constexpr int16_t REMINDER_BLOCK = REMINDER_HEIGHT + BAND_GAP;  // 44

// Agenda rows and the space that must remain below the last one.
constexpr int16_t ROW_HEIGHT = 20;
constexpr int16_t RUNNING_ROW_HEIGHT = 24;
constexpr int16_t ROW_GAP = 4;
constexpr int16_t AGENDA_SLACK = 20;

// Column widths inside a row.
constexpr int16_t TODAY_TIME_WIDTH = 112;
constexpr int16_t AHEAD_DAY_WIDTH = 62;
constexpr int16_t AHEAD_TIME_WIDTH = 46;
constexpr int16_t ROW_GUTTER = 12;

// The RUNNING day strip.
constexpr int16_t DAY_BOX_WIDTH = 20;
constexpr int16_t DAY_BOX_HEIGHT = 12;
constexpr int16_t DAY_BOX_GAP = 4;
constexpr int16_t DAY_BOX_COUNT = 4;

// The hourly strip: five columns filling the content width exactly.
constexpr int16_t HOUR_COLUMNS = 5;
constexpr int16_t HOUR_COLUMN_WIDTH =
    CONTENT_WIDTH / HOUR_COLUMNS;  // 88 by default
static_assert(
    CONTENT_WIDTH % HOUR_COLUMNS == 0,
    "DISPLAY_PADDING_LEFT + DISPLAY_PADDING_RIGHT must be a multiple of 5");
constexpr int16_t HOUR_ICON_SIZE = 24;
constexpr int16_t HERO_ICON_SIZE = 44;

// Precipitation probability is suppressed below this, so dry hours stay empty.
constexpr int16_t PROBABILITY_FLOOR = 10;

// The five faces. Tracking is the design's em value at the face's size:
// 0.16em of 13px rounds to 2, the 0.06-0.1em variants to 1, and the hero
// numerals are tightened by 0.04em of 56px.
constexpr TextStyle STYLE_HERO{&ArchivoBlack56, -2};
constexpr TextStyle STYLE_TITLE{&ArchivoBold19, 0};
constexpr TextStyle STYLE_SUBTITLE{&ArchivoSemiBold16, 0};
constexpr TextStyle STYLE_TIME{&MonoSemiBold15, 0};
constexpr TextStyle STYLE_META{&MonoRegular13, 0};
constexpr TextStyle STYLE_META_WIDE{&MonoRegular13, 1};
constexpr TextStyle STYLE_LABEL{&MonoRegular13, 2};

}  // namespace ui
