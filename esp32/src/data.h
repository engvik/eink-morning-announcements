#pragma once

#include "http.h"
#include "ui/view_model.h"

// Fetches from the endpoints and decodes them into the model.
//
// Returns false without meta, so the panel can keep its last good render.
bool fetchDisplayData(BackendClient& backend, ui::DisplayModel& model);
