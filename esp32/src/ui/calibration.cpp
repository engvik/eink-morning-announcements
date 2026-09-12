#include "ui/calibration.h"

#include <cstdio>

#include "ui/text.h"
#include "ui/theme.h"

namespace ui {

namespace {

// Ticks every 5px, the step padding is set in, and a number every 20.
constexpr int16_t RULER_DEPTH = 150;
constexpr int16_t TICK_STEP = 5;
constexpr int16_t LABEL_STEP = 20;
constexpr int16_t TICK_SHORT = 6;
constexpr int16_t TICK_LONG = 14;
constexpr int16_t LABEL_GAP = 4;
constexpr int16_t DIGIT_HEIGHT = 9;
constexpr int16_t NAME_GAP = 24;

int16_t tickLength(int16_t depth) {
  return depth % LABEL_STEP == 0 ? TICK_LONG : TICK_SHORT;
}

// The tick at depth d lands on the first or last row a padding of d leaves for
// content, so a visible tick means that padding is visible too.
void drawTopOrBottom(Adafruit_GFX& gfx, bool top, const char* name) {
  const int16_t x = PANEL_WIDTH / 2;
  const auto row = [top](int16_t depth) {
    return static_cast<int16_t>(top ? depth : PANEL_HEIGHT - 1 - depth);
  };

  gfx.drawFastVLine(x, top ? 0 : row(RULER_DEPTH), RULER_DEPTH + 1, INK);

  for (int16_t depth = TICK_STEP; depth <= RULER_DEPTH; depth += TICK_STEP) {
    const int16_t length = tickLength(depth);
    gfx.drawFastHLine(x - length, row(depth), length, INK);

    if (depth % LABEL_STEP == 0) {
      char label[4];
      snprintf(label, sizeof(label), "%d", depth);
      drawLeft(gfx, STYLE_META, x + LABEL_GAP, row(depth) + DIGIT_HEIGHT / 2,
               label);
    }
  }

  const int16_t nameBaseline = top ? row(RULER_DEPTH) + NAME_GAP + DIGIT_HEIGHT
                                   : row(RULER_DEPTH) - NAME_GAP;
  drawCentred(gfx, STYLE_LABEL, x, nameBaseline, name);
}

// Numbers alternate above and below, as three digits are wider than 20px.
void drawLeftOrRight(Adafruit_GFX& gfx, bool left, const char* name) {
  const int16_t y = PANEL_HEIGHT / 2;
  const auto column = [left](int16_t depth) {
    return static_cast<int16_t>(left ? depth : PANEL_WIDTH - 1 - depth);
  };

  gfx.drawFastHLine(left ? 0 : column(RULER_DEPTH), y, RULER_DEPTH + 1, INK);

  for (int16_t depth = TICK_STEP; depth <= RULER_DEPTH; depth += TICK_STEP) {
    gfx.drawFastVLine(column(depth), y, tickLength(depth), INK);

    if (depth % LABEL_STEP == 0) {
      char label[4];
      snprintf(label, sizeof(label), "%d", depth);
      const bool above = (depth / LABEL_STEP) % 2 == 1;
      const int16_t baseline =
          above ? y - LABEL_GAP : y + TICK_LONG + LABEL_GAP + DIGIT_HEIGHT;
      drawCentred(gfx, STYLE_META, column(depth), baseline, label);
    }
  }

  const int16_t nameBaseline = y + DIGIT_HEIGHT / 2;
  if (left) {
    drawLeft(gfx, STYLE_LABEL, column(RULER_DEPTH) + NAME_GAP, nameBaseline,
             name);
  } else {
    drawRight(gfx, STYLE_LABEL, column(RULER_DEPTH) - NAME_GAP, nameBaseline,
              name);
  }
}

}  // namespace

void drawCalibration(Adafruit_GFX& gfx) {
  gfx.fillScreen(PAPER);

  drawTopOrBottom(gfx, true, "TOP");
  drawTopOrBottom(gfx, false, "BOTTOM");
  drawLeftOrRight(gfx, true, "LEFT");
  drawLeftOrRight(gfx, false, "RIGHT");

  const int16_t centre = PANEL_WIDTH / 2;
  drawCentred(gfx, STYLE_TITLE, centre, 272, "Frame calibration");
  drawCentred(gfx, STYLE_META, centre, 300, "Note the first tick");
  drawCentred(gfx, STYLE_META, centre, 318, "visible on each edge");
}

}  // namespace ui
