#include "eink_display.h"

#include <GxEPD2_BW.h>

#include "config.h"
#include "ui/panel.h"

namespace {

// A full framebuffer rather than paged drawing: the page height matches the
// panel, so the firstPage/nextPage loop runs exactly once.
GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> panel(
    GxEPD2_750_T7(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY));

}  // namespace

void renderDisplay(const ui::DisplayModel& model) {
  panel.init(SERIAL_BAUD, true, 2, false);
  panel.setRotation(1);
  panel.setFullWindow();
  panel.firstPage();

  do {
    ui::drawPanel(panel, model);
  } while (panel.nextPage());

  panel.hibernate();
}
