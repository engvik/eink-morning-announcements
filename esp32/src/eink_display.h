#pragma once

#include "ui/view_model.h"

// A thin wrapper over the panel driver. Everything about how the display looks
// lives in ui/panel.cpp, which draws against Adafruit_GFX and so renders
// identically here and in the host preview.
class EinkDisplay {
 public:
  void init();
  void draw(const ui::DisplayModel& model);
  void hibernate();
};
