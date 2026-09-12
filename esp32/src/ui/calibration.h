#pragma once

#include <Adafruit_GFX.h>

namespace ui {

// A ruler at the middle of each panel edge, for sizing DISPLAY_PADDING_*. The
// first tick the frame's mat leaves visible on an edge is how much it hides.
void drawCalibration(Adafruit_GFX& gfx);

}  // namespace ui
