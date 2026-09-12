#pragma once

#include <Adafruit_GFX.h>

#include "ui/view_model.h"

namespace ui {

// The whole page. Device and preview both draw through this.
void drawPanel(Adafruit_GFX& gfx, const DisplayModel& model);

}  // namespace ui
