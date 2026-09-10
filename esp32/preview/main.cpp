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
constexpr const char *WEATHER = R"({
  "forecasts": [{"time": "2026-09-26T06:00:00+02:00",
    "instant": {"air_temperature": 21.3, "apparent_air_temperature": 19.2,
                "wind_speed": 4.1, "wind_speed_of_gust": 8.7},
    "one_hour": {"symbol_code": "partlycloudy_day"}}],
  "days": [{"date": "2026-09-26", "air_temperature_min": 12.4,
            "air_temperature_max": 21.0, "ultraviolet_index_max": 3.2,
            "precipitation_amount": 2.7}]})";

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
  ui::decodeWeather(model, WEATHER);

  GFXcanvas1 canvas(ui::PANEL_WIDTH, ui::PANEL_HEIGHT);
  canvas.fillScreen(ui::PAPER);

  int16_t y = ui::drawHeader(canvas, model, ui::PADDING);
  y = ui::drawReminder(canvas, model, y);
  y = ui::drawWeather(canvas, model, y);

  guide(canvas, y, "next band");

  char line[80];
  snprintf(line, sizeof(line), "next y = %d, design says 228", y);
  note(canvas, y + 26, line);

  // The longest condition MET publishes, against the widest temperatures.
  ui::DisplayModel wide = model;
  wide.weather.condition = "Heavy sleet showers and thunder";
  wide.weather.temperature = -18;
  wide.weather.low = -24;
  wide.weather.feels = -31;
  wide.weather.wind = 12;
  wide.weather.gust = 28;
  wide.weather.precipitation = 14.5f;

  int16_t z = ui::drawHeader(canvas, wide, y + 40);
  z = ui::drawReminder(canvas, wide, z);
  z = ui::drawWeather(canvas, wide, z);
  note(canvas, z + 26, "longest condition, negative temperatures");

  // A failed weather fetch keeps the band's height so nothing below shifts.
  ui::DisplayModel blank = model;
  blank.weather.valid = false;

  int16_t w = ui::drawWeather(canvas, blank, z + 40);
  guide(canvas, w, "after empty band");
  note(canvas, w + 26, "no weather, band keeps its height");

  if (!png::write(out, canvas.getBuffer(), ui::PANEL_WIDTH, ui::PANEL_HEIGHT)) {
    fprintf(stderr, "could not write %s\n", out.c_str());
    return 1;
  }

  printf("wrote %s (%dx%d)\n", out.c_str(), ui::PANEL_WIDTH, ui::PANEL_HEIGHT);

  return 0;
}
