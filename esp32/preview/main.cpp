// Renders the display to a PNG on the host, so layout work does not need a
// flash cycle. The canvas is the same size as the panel and drawing goes
// through Adafruit_GFX, exactly as it does on the device.
#include <Adafruit_GFX.h>

#include <cstdio>
#include <string>

#include "icons/met_icons_black_24x24.h"
#include "icons/met_icons_black_44x44.h"
#include "png.h"
#include "ui/text.h"
#include "ui/theme.h"

namespace {

struct Sample {
  const unsigned char *small;
  const unsigned char *large;
};

#define SAMPLE(name)                                     \
  {                                                      \
    met_bitmap_black_24x24_##name, met_bitmap_black_44x44_##name \
  }

// A spread across the set: clear, partial, overcast, wet, frozen, night.
const Sample SAMPLES[] = {
    SAMPLE(clearsky_day),  SAMPLE(fair_day),   SAMPLE(partlycloudy_day),
    SAMPLE(cloudy),        SAMPLE(lightrain),  SAMPLE(rain),
    SAMPLE(heavyrain),     SAMPLE(sleet),      SAMPLE(snow),
    SAMPLE(heavysnow),     SAMPLE(fog),        SAMPLE(rainandthunder),
    SAMPLE(clearsky_night), SAMPLE(partlycloudy_night),
};

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

  ui::drawLeft(canvas, ui::STYLE_LABEL, ui::CONTENT_X, 34, "WEATHER ICONS");
  rule(canvas, 44);

  // Hero size, as the weather band uses it.
  caption(canvas, 66, "44x44 hero");

  int16_t x = ui::CONTENT_X;
  for (const Sample &sample : SAMPLES) {
    if (x + ui::HERO_ICON_SIZE > ui::CONTENT_X + ui::CONTENT_WIDTH) break;

    canvas.drawBitmap(x, 76, sample.large, ui::HERO_ICON_SIZE,
                      ui::HERO_ICON_SIZE, ui::INK);
    x += ui::HERO_ICON_SIZE + 6;
  }

  rule(canvas, 134);

  // The hourly strip geometry: five 88px columns with dividers between them.
  caption(canvas, 156, "24x24 strip, five 88px columns");

  const int16_t top = 168;
  for (int16_t column = 0; column < ui::HOUR_COLUMNS; column++) {
    const int16_t left = ui::CONTENT_X + column * ui::HOUR_COLUMN_WIDTH;
    const int16_t centre = left + ui::HOUR_COLUMN_WIDTH / 2;

    if (column > 0) {
      canvas.drawFastVLine(left, top, 112, ui::INK);
    }

    char hour[3];
    snprintf(hour, sizeof(hour), "%02d", 8 + column * 2);

    ui::drawCentred(canvas, ui::STYLE_META_WIDE, centre, top + 16, hour);
    canvas.drawBitmap(centre - ui::HOUR_ICON_SIZE / 2, top + 24,
                      SAMPLES[column].small, ui::HOUR_ICON_SIZE,
                      ui::HOUR_ICON_SIZE, ui::INK);
    ui::drawCentred(canvas, ui::STYLE_TITLE, centre, top + 74, "16");
    ui::drawCentred(canvas, ui::STYLE_META, centre, top + 96,
                    column < 2 ? "-" : "0.4");
    ui::drawCentred(canvas, ui::STYLE_META_WIDE, centre, top + 112,
                    column < 2 ? "" : "30%");
  }

  rule(canvas, 296);

  // A wider sweep at strip size, to spot any icon that packs wrong.
  caption(canvas, 318, "24x24 sweep");

  int16_t gx = ui::CONTENT_X;
  int16_t gy = 328;
  for (const Sample &sample : SAMPLES) {
    canvas.drawBitmap(gx, gy, sample.small, ui::HOUR_ICON_SIZE,
                      ui::HOUR_ICON_SIZE, ui::INK);

    gx += ui::HOUR_ICON_SIZE + 6;
    if (gx + ui::HOUR_ICON_SIZE > ui::CONTENT_X + ui::CONTENT_WIDTH) {
      gx = ui::CONTENT_X;
      gy += ui::HOUR_ICON_SIZE + 6;
    }
  }

  rule(canvas, 400);

  // Both sizes side by side, so the detail lost at 24 is visible.
  caption(canvas, 422, "44 and 24 compared");

  int16_t cx = ui::CONTENT_X;
  for (const Sample &sample : SAMPLES) {
    if (cx + ui::HERO_ICON_SIZE > ui::CONTENT_X + ui::CONTENT_WIDTH) break;

    canvas.drawBitmap(cx, 432, sample.large, ui::HERO_ICON_SIZE,
                      ui::HERO_ICON_SIZE, ui::INK);
    canvas.drawBitmap(cx + (ui::HERO_ICON_SIZE - ui::HOUR_ICON_SIZE) / 2, 482,
                      sample.small, ui::HOUR_ICON_SIZE, ui::HOUR_ICON_SIZE,
                      ui::INK);
    cx += ui::HERO_ICON_SIZE + 10;
  }

  if (!png::write(out, canvas.getBuffer(), ui::PANEL_WIDTH, ui::PANEL_HEIGHT)) {
    fprintf(stderr, "could not write %s\n", out.c_str());
    return 1;
  }

  printf("wrote %s (%dx%d)\n", out.c_str(), ui::PANEL_WIDTH, ui::PANEL_HEIGHT);

  return 0;
}
