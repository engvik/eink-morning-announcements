#include "eink_display.h"

#include <GxEPD2_BW.h>

#include "config.h"
#include "ui/calibration.h"
#include "ui/panel.h"
#include "ui/theme.h"

static_assert(ui::INK == GxEPD_BLACK && ui::PAPER == GxEPD_WHITE,
              "ui::INK and ui::PAPER must match GxEPD2's colours");

namespace {

// A full framebuffer rather than paged drawing: the page height matches the
// panel, so the firstPage/nextPage loop runs exactly once.
GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> panel(
    GxEPD2_750_T7(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY));

template <typename Draw>
void render(Draw draw) {
  panel.init(SERIAL_BAUD, true, 2, false);
  panel.setRotation(1);
  panel.setFullWindow();
  panel.firstPage();

  do {
    draw(panel);
  } while (panel.nextPage());

  panel.hibernate();
}

}  // namespace

void renderDisplay(const ui::DisplayModel& model) {
  render([&model](Adafruit_GFX& gfx) { ui::drawPanel(gfx, model); });
}

void renderCalibration() { render(ui::drawCalibration); }
