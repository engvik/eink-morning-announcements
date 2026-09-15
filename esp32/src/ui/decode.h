#pragma once

#include "ui/view_model.h"

// Turns the backend's JSON into the model the panel draws from.
//
// Each endpoint decodes independently and tolerates a null or malformed body,
// leaving its part of the model empty rather than failing the whole decode.
namespace ui {

// Clears the model. Call before decoding, so a failed fetch leaves empty
// fields rather than whatever the last wake put there.
void clear(DisplayModel& model);

// Decode in this order: meta establishes the current date that the calendar
// sorts against, and weather fills the day buckets the AHEAD rows summarise.
void decodeMeta(DisplayModel& model, const char* json);
void decodeWeather(DisplayModel& model, const char* json);
void decodeMessage(DisplayModel& model, const char* json);
void decodeCalendar(DisplayModel& model, const char* json);
void decodeNews(DisplayModel& model, const char* json);

}  // namespace ui
