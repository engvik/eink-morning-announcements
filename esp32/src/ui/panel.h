#pragma once

#include <Adafruit_GFX.h>

#include "ui/view_model.h"

// The panel is a stack of fixed bands, drawn top down. Each takes the y it
// starts at and returns the y the next band starts at, so the reminder can
// collapse without every band below it needing to know.
namespace ui {

int16_t drawHeader(Adafruit_GFX& gfx, const DisplayModel& model, int16_t top);
int16_t drawReminder(Adafruit_GFX& gfx, const DisplayModel& model, int16_t top);
int16_t drawWeather(Adafruit_GFX& gfx, const DisplayModel& model, int16_t top);
int16_t drawHourly(Adafruit_GFX& gfx, const DisplayModel& model, int16_t top);

}  // namespace ui
