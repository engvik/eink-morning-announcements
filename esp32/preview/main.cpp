// Renders the display to a PNG on the host, so layout work does not need a
// flash cycle. The canvas is the same size as the panel and drawing goes
// through Adafruit_GFX, exactly as it does on the device.
#include <Adafruit_GFX.h>

#include <cstdio>
#include <string>

#include "png.h"
#include "ui/text.h"
#include "ui/theme.h"

namespace {

void rule(GFXcanvas1 &canvas, int16_t y) {
  canvas.drawFastHLine(ui::CONTENT_X, y, ui::CONTENT_WIDTH, ui::INK);
}

void caption(GFXcanvas1 &canvas, int16_t baseline, const char *text) {
  ui::drawLeft(canvas, ui::STYLE_META, ui::CONTENT_X, baseline, text);
}

}  // namespace

int main(int argc, char **argv) {
  const std::string out = argc > 1 ? argv[1] : "preview.png";

  GFXcanvas1 canvas(ui::PANEL_WIDTH, ui::PANEL_HEIGHT);
  canvas.fillScreen(ui::PAPER);

  const int16_t right = ui::CONTENT_X + ui::CONTENT_WIDTH;

  ui::drawLeft(canvas, ui::STYLE_LABEL, ui::CONTENT_X, 34, "TEXT HELPERS");
  rule(canvas, 44);

  // UTF-8 written as ordinary strings, no escapes.
  ui::drawLeft(canvas, ui::STYLE_TITLE, ui::CONTENT_X, 82, "Blåbær på Vestlandet");
  caption(canvas, 102, "utf-8 title, ÆØÅ æøå");

  ui::drawLeft(canvas, ui::STYLE_META, ui::CONTENT_X, 132,
               "LOW 12° · FEELS 19° - 2.7 MM - 30%");
  caption(canvas, 152, "symbols: degree, middot, hyphen, ellipsis");

  rule(canvas, 166);

  // Tracking. The same string at three tracking values.
  ui::drawLeft(canvas, ui::STYLE_META, ui::CONTENT_X, 196, "HOUR BY HOUR");
  caption(canvas, 214, "STYLE_META, tracking 0");

  ui::drawLeft(canvas, ui::STYLE_META_WIDE, ui::CONTENT_X, 240, "HOUR BY HOUR");
  caption(canvas, 258, "STYLE_META_WIDE, tracking 1");

  ui::drawLeft(canvas, ui::STYLE_LABEL, ui::CONTENT_X, 284, "HOUR BY HOUR");
  caption(canvas, 302, "STYLE_LABEL, tracking 2");

  rule(canvas, 316);

  // Alignment. A tick marks each anchor so drift is visible.
  ui::drawLeft(canvas, ui::STYLE_TIME, ui::CONTENT_X, 346, "left edge");
  canvas.drawFastVLine(ui::CONTENT_X, 350, 6, ui::INK);

  ui::drawRight(canvas, ui::STYLE_TIME, right, 372, "right edge");
  canvas.drawFastVLine(right - 1, 376, 6, ui::INK);

  const int16_t centre = ui::CONTENT_X + ui::CONTENT_WIDTH / 2;
  ui::drawCentred(canvas, ui::STYLE_TIME, centre, 398, "centred");
  canvas.drawFastVLine(centre, 402, 6, ui::INK);

  caption(canvas, 420, "alignment, ticks mark the anchors");

  rule(canvas, 434);

  // Truncation. A column that most of these overrun.
  constexpr int16_t COLUMN = 200;
  canvas.drawFastVLine(ui::CONTENT_X + COLUMN, 452, 92, ui::INK);

  const char *samples[] = {
      "Fits easily",
      "This one is right at the edge",
      "A considerably longer string that cannot fit the column",
      "Ærlig ørret på åsen i overmorgen ved sjøen",
  };

  int16_t y = 466;
  for (const char *sample : samples) {
    ui::drawTruncated(canvas, ui::STYLE_SUBTITLE, ui::CONTENT_X, y, COLUMN, sample);
    y += 22;
  }

  caption(canvas, 562, "truncation to the rule, ellipsis when clipped");

  rule(canvas, 576);

  // Measurement must agree with what was drawn.
  const char *probe = "Måler 123 °";
  const int16_t width = ui::measure(ui::STYLE_TITLE, probe);

  ui::drawLeft(canvas, ui::STYLE_TITLE, ui::CONTENT_X, 610, probe);
  canvas.drawRect(ui::CONTENT_X - 1, 592, width + 2, 22, ui::INK);

  char line[80];
  snprintf(line, sizeof(line), "measure() = %d px, box drawn to match", width);
  caption(canvas, 634, line);

  // Baselines on the 20px agenda rhythm, to check row spacing.
  rule(canvas, 650);
  ui::drawLeft(canvas, ui::STYLE_LABEL, ui::CONTENT_X, 676, "20PX ROWS");

  for (int16_t row = 0; row < 5; row++) {
    const int16_t baseline = 700 + row * ui::ROW_HEIGHT;

    ui::drawLeft(canvas, ui::STYLE_TIME, ui::CONTENT_X, baseline, "00:00");
    ui::drawTruncated(canvas, ui::STYLE_TITLE, ui::CONTENT_X + ui::TODAY_TIME_WIDTH,
                      baseline, ui::CONTENT_WIDTH - ui::TODAY_TIME_WIDTH,
                      "Row title at the agenda rhythm");
  }

  if (!png::write(out, canvas.getBuffer(), ui::PANEL_WIDTH, ui::PANEL_HEIGHT)) {
    fprintf(stderr, "could not write %s\n", out.c_str());
    return 1;
  }

  printf("wrote %s (%dx%d)\n", out.c_str(), ui::PANEL_WIDTH, ui::PANEL_HEIGHT);

  return 0;
}
