#pragma once

#include "ui/view_model.h"

// Draws the model on the panel, then hibernates it.
void renderDisplay(const ui::DisplayModel& model);

// Draws the frame calibration rulers instead, then hibernates the panel.
void renderCalibration();
