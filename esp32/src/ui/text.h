#pragma once

#include <Adafruit_GFX.h>

#include "ui/theme.h"

// Text drawing for the panel. Everything takes UTF-8 and a baseline, because
// that is what the design specifies and what Adafruit_GFX draws from.
//
// The generated fonts keep the non-ASCII characters the design needs in a
// private index space above ASCII, so these helpers translate codepoints on the
// way through. Drawing goes glyph by glyph, which is what makes letter-spacing
// possible and keeps measurement and drawing in agreement.
namespace ui {

// Longest run of glyphs any single string may draw. Titles are truncated to fit
// their column long before this matters.
constexpr size_t MAX_GLYPHS = 128;

// Width in pixels, including tracking between glyphs.
int16_t measure(const TextStyle& style, const char* utf8);

int16_t measureGlyph(const TextStyle& style, uint32_t codepoint);

// Colour is a parameter because the reminder bar draws paper on ink; every
// other band draws ink on paper.
void drawLeft(Adafruit_GFX& gfx, const TextStyle& style, int16_t x,
              int16_t baseline, const char* utf8,
              uint16_t colour = INK);

void drawRight(Adafruit_GFX& gfx, const TextStyle& style, int16_t right,
               int16_t baseline, const char* utf8,
               uint16_t colour = INK);

void drawCentred(Adafruit_GFX& gfx, const TextStyle& style, int16_t centre,
                 int16_t baseline, const char* utf8,
                 uint16_t colour = INK);

// Draws as much as fits in width, ending with an ellipsis when clipped. The
// design never wraps, so this is the only overflow behaviour.
void drawTruncated(Adafruit_GFX& gfx, const TextStyle& style, int16_t x,
                   int16_t baseline, int16_t width, const char* utf8,
                   uint16_t colour = INK);

}  // namespace ui
