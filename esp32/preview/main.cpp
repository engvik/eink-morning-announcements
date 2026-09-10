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
  // Dotted rule marking a band boundary, with the expected y beside it.
  for (int16_t x = 0; x < ui::PANEL_WIDTH; x += 6) {
    canvas.drawFastHLine(x, y, 2, ui::INK);
  }

  char label[48];
  snprintf(label, sizeof(label), "%d %s", y, note);
  ui::drawLeft(canvas, ui::STYLE_META, ui::PANEL_WIDTH - 150, y - 4, label);
}

}  // namespace

int main(int argc, char **argv) {
  const std::string out = argc > 1 ? argv[1] : "preview.png";

  ui::DisplayModel model;
  ui::clear(model);
  ui::decodeMeta(model, META);

  GFXcanvas1 canvas(ui::PANEL_WIDTH, ui::PANEL_HEIGHT);
  canvas.fillScreen(ui::PAPER);

  const int16_t next = ui::drawHeader(canvas, model, ui::PADDING);

  // The design puts the band at 20..84 with its rule as the bottom edge.
  guide(canvas, ui::PADDING, "band top");
  guide(canvas, next, "band bottom, next band starts");

  ui::drawLeft(canvas, ui::STYLE_META, ui::CONTENT_X, next + 30,
               "header band, compared against artboard 4C");

  char note[64];
  snprintf(note, sizeof(note), "returned next y = %d, expected %d", next,
           ui::PADDING + ui::HEADER_HEIGHT);
  ui::drawLeft(canvas, ui::STYLE_META, ui::CONTENT_X, next + 50, note);

  if (!png::write(out, canvas.getBuffer(), ui::PANEL_WIDTH, ui::PANEL_HEIGHT)) {
    fprintf(stderr, "could not write %s\n", out.c_str());
    return 1;
  }

  printf("wrote %s (%dx%d)\n", out.c_str(), ui::PANEL_WIDTH, ui::PANEL_HEIGHT);

  return 0;
}
