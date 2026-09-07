// Renders the display to a PNG on the host, so layout work does not need a
// flash cycle. The canvas is the same size as the panel and drawing goes
// through Adafruit_GFX, exactly as it does on the device.
#include <Adafruit_GFX.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>

#include <cstdio>
#include <string>

#include "png.h"

namespace {

// The panel is 800x480 rotated to portrait.
constexpr int16_t kWidth = 480;
constexpr int16_t kHeight = 800;

// A set bit is ink. On the device this maps to GxEPD_BLACK.
constexpr uint16_t kInk = 1;
constexpr uint16_t kPaper = 0;

}  // namespace

int main(int argc, char **argv) {
  const std::string out = argc > 1 ? argv[1] : "preview.png";

  GFXcanvas1 canvas(kWidth, kHeight);
  canvas.fillScreen(kPaper);
  canvas.setTextColor(kInk);

  // Placeholder content until the panel bands land. Enough to prove the
  // pipeline: geometry, fonts, text metrics and the file writer.
  canvas.drawRect(0, 0, kWidth, kHeight, kInk);

  canvas.setFont(&FreeMonoBold12pt7b);
  canvas.setCursor(20, 60);
  canvas.print("eink preview");

  canvas.drawFastHLine(20, 84, kWidth - 40, kInk);

  canvas.setFont(&FreeMono9pt7b);
  canvas.setCursor(20, 120);
  canvas.print("480x800, 1-bit");

  // A ruler down the left edge, to check the PNG is not sheared or offset.
  for (int16_t y = 0; y < kHeight; y += 100) {
    canvas.drawFastHLine(0, y, y % 500 == 0 ? 24 : 12, kInk);
  }

  if (!png::write(out, canvas.getBuffer(), kWidth, kHeight)) {
    fprintf(stderr, "could not write %s\n", out.c_str());
    return 1;
  }

  printf("wrote %s (%dx%d)\n", out.c_str(), kWidth, kHeight);

  return 0;
}
