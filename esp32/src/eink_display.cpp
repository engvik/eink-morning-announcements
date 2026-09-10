#include "eink_display.h"

#include <GxEPD2_BW.h>

#include "config.h"
#include "ui/panel.h"
#include "ui/theme.h"

namespace {

// A full framebuffer rather than paged drawing: the page height matches the
// panel, so the firstPage/nextPage loop runs exactly once.
GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> panel(
    GxEPD2_750_T7(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY));

}  // namespace

void EinkDisplay::init() {
  panel.init(SERIAL_BAUD, true, 2, false);
  panel.setRotation(1);
}

void EinkDisplay::draw(const ui::DisplayModel& model) {
  panel.setFullWindow();
  panel.firstPage();

  do {
    panel.fillScreen(ui::PAPER);

    int16_t y = ui::drawHeader(panel, model, ui::PADDING);
    y = ui::drawReminder(panel, model, y);
    y = ui::drawWeather(panel, model, y);
    y = ui::drawHourly(panel, model, y);

    ui::drawAgenda(panel, model, y);
    ui::drawFooter(panel, model);
  } while (panel.nextPage());
}

void EinkDisplay::hibernate() { panel.hibernate(); }
