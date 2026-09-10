// Renders the display to a PNG on the host, so layout work does not need a
// flash cycle. The canvas is the same size as the panel and drawing goes
// through Adafruit_GFX, exactly as it does on the device.
#include <Adafruit_GFX.h>

#include <cstdio>
#include <cstring>
#include <string>

#include "png.h"
#include "ui/decode.h"
#include "ui/panel.h"
#include "ui/text.h"
#include "ui/theme.h"

namespace {

// Stand-in data with the shape the backend produces. Deliberately generic.
constexpr const char *META = R"({
  "today": "Wednesday", "month": "September", "date": 26, "week": 35,
  "now": "2026-09-26T06:00:00+02:00",
  "sunrise": "2026-09-26T05:52:00+02:00",
  "sunset": "2026-09-26T20:41:00+02:00"})";

void guide(GFXcanvas1 &canvas, int16_t y, const char *note) {
  for (int16_t x = 0; x < ui::PANEL_WIDTH; x += 6) {
    canvas.drawFastHLine(x, y, 2, ui::INK);
  }

  char label[56];
  snprintf(label, sizeof(label), "%d %s", y, note);
  ui::drawRight(canvas, ui::STYLE_META, ui::PANEL_WIDTH - 4, y - 4, label);
}

void note(GFXcanvas1 &canvas, int16_t baseline, const char *text) {
  ui::drawLeft(canvas, ui::STYLE_META, ui::CONTENT_X, baseline, text);
}

}  // namespace

int main(int argc, char **argv) {
  const std::string out = argc > 1 ? argv[1] : "preview.png";

  ui::DisplayModel model;
  ui::clear(model);
  ui::decodeMeta(model, META);
  ui::decodeMessage(model, R"({"message":"Bins out before 07:00"})");

  GFXcanvas1 canvas(ui::PANEL_WIDTH, ui::PANEL_HEIGHT);
  canvas.fillScreen(ui::PAPER);

  // With a message: header, then the bar.
  int16_t y = ui::drawHeader(canvas, model, ui::PADDING);
  y = ui::drawReminder(canvas, model, y);

  guide(canvas, y, "next band");
  note(canvas, y + 26, "with a message");

  char line[72];
  snprintf(line, sizeof(line), "next y = %d, design says 128", y);
  note(canvas, y + 44, line);

  // A message long enough to need truncating inside the bar.
  ui::DisplayModel wide = model;
  std::strncpy(wide.reminder,
               "A reminder long enough that it cannot possibly fit the bar",
               sizeof(wide.reminder) - 1);

  int16_t z = ui::drawHeader(canvas, wide, y + 70);
  z = ui::drawReminder(canvas, wide, z);
  note(canvas, z + 26, "long message, truncated to the bar");

  // No message at all: the bar and its gap disappear.
  ui::DisplayModel quiet = model;
  quiet.reminder[0] = '\0';

  const int16_t collapsedTop = z + 50;
  int16_t w = ui::drawHeader(canvas, quiet, collapsedTop);
  const int16_t afterHeader = w;
  w = ui::drawReminder(canvas, quiet, w);

  guide(canvas, w, "next band");
  note(canvas, w + 26, "no message, band collapses");

  snprintf(line, sizeof(line), "header ended %d, reminder returned %d, saved %d",
           afterHeader, w, ui::REMINDER_BLOCK);
  note(canvas, w + 44, line);

  if (!png::write(out, canvas.getBuffer(), ui::PANEL_WIDTH, ui::PANEL_HEIGHT)) {
    fprintf(stderr, "could not write %s\n", out.c_str());
    return 1;
  }

  printf("wrote %s (%dx%d)\n", out.c_str(), ui::PANEL_WIDTH, ui::PANEL_HEIGHT);

  return 0;
}
