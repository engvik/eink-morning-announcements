#pragma once

#include <Adafruit_GFX.h>

#include "fonts/ArchivoBlack56.h"
#include "fonts/ArchivoBold19.h"
#include "fonts/ArchivoSemiBold16.h"
#include "fonts/MonoRegular13.h"
#include "fonts/MonoSemiBold15.h"

#ifndef PREVIEW_HOST
#include <GxEPD2_BW.h>
#endif

namespace ui {

// GxEPD2 and GFXcanvas1 disagree about what a colour value means: the panel
// takes GxEPD_BLACK, which is zero, while the canvas sets a bit for any
// non-zero value. Drawing code says ink or paper and stays portable.
#ifdef PREVIEW_HOST
constexpr uint16_t INK = 1;
constexpr uint16_t PAPER = 0;
#else
constexpr uint16_t INK = GxEPD_BLACK;
constexpr uint16_t PAPER = GxEPD_WHITE;
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

constexpr int16_t PADDING = 20;
constexpr int16_t CONTENT_X = PADDING;
constexpr int16_t CONTENT_WIDTH = PANEL_WIDTH - PADDING * 2;  // 440

constexpr int16_t RULE_THIN = 1;
constexpr int16_t RULE_THICK = 3;

// Band heights. Borders sit inside these, as they do in the design.
constexpr int16_t HEADER_HEIGHT = 64;
constexpr int16_t REMINDER_HEIGHT = 32;
constexpr int16_t WEATHER_HEIGHT = 88;
constexpr int16_t HOURLY_HEIGHT = 124;
constexpr int16_t FOOTER_HEIGHT = 28;

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
constexpr int16_t HOUR_COLUMN_WIDTH = CONTENT_WIDTH / HOUR_COLUMNS;  // 88
constexpr int16_t HOUR_ICON_SIZE = 24;
constexpr int16_t HERO_ICON_SIZE = 44;

// Precipitation probability is suppressed below this, so dry hours stay empty.
constexpr float PROBABILITY_FLOOR = 10.0f;

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
