#include "ui/text.h"

// The ESP32 core supplies this. On the host, read the pointer as a pointer
// rather than through pgm_read_dword, which puns it via unsigned long and
// trips strict aliasing.
#ifndef pgm_read_pointer
#define pgm_read_pointer(addr) (*reinterpret_cast<void* const*>(addr))
#endif

namespace ui {
namespace {

struct Mapping {
  uint32_t codepoint;
  uint8_t index;
};

// Must match EXTRAS in tools/genfonts.py, in the same order.
constexpr Mapping EXTRAS[] = {
    {0x00E6, 0x7F},  // æ
    {0x00F8, 0x80},  // ø
    {0x00E5, 0x81},  // å
    {0x00C6, 0x82},  // Æ
    {0x00D8, 0x83},  // Ø
    {0x00C5, 0x84},  // Å
    {0x00B7, 0x85},  // ·
    {0x00B0, 0x86},  // °
    {0x2026, 0x87},  // ellipsis
};

constexpr uint32_t ELLIPSIS = 0x2026;

// Decodes one UTF-8 sequence and advances the cursor. Malformed bytes are
// consumed and reported as zero rather than derailing the rest of the string.
uint32_t decode(const char*& cursor) {
  const uint8_t lead = static_cast<uint8_t>(*cursor++);

  if (lead < 0x80) {
    return lead;
  }

  int extra;
  uint32_t codepoint;

  if ((lead & 0xE0) == 0xC0) {
    extra = 1;
    codepoint = lead & 0x1F;
  } else if ((lead & 0xF0) == 0xE0) {
    extra = 2;
    codepoint = lead & 0x0F;
  } else if ((lead & 0xF8) == 0xF0) {
    extra = 3;
    codepoint = lead & 0x07;
  } else {
    return 0;  // stray continuation byte
  }

  while (extra-- > 0) {
    const uint8_t next = static_cast<uint8_t>(*cursor);

    if ((next & 0xC0) != 0x80) {
      return 0;  // truncated sequence, leave the cursor on the bad byte
    }

    codepoint = (codepoint << 6) | (next & 0x3F);
    cursor++;
  }

  return codepoint;
}

// Maps a codepoint into the font's index space. Returns 0 when the font has no
// glyph for it, which callers skip.
uint8_t glyphIndex(uint32_t codepoint) {
  if (codepoint >= 0x20 && codepoint < 0x7F) {
    return static_cast<uint8_t>(codepoint);
  }

  for (const Mapping& mapping : EXTRAS) {
    if (mapping.codepoint == codepoint) {
      return mapping.index;
    }
  }

  return 0;
}

int16_t advanceOf(const GFXfont* font, uint8_t index) {
  const uint8_t first = pgm_read_byte(&font->first);
  const uint8_t last = pgm_read_byte(&font->last);

  if (index < first || index > last) {
    return 0;
  }

  const GFXglyph* glyphs =
      static_cast<GFXglyph*>(pgm_read_pointer(&font->glyph));

  return static_cast<int16_t>(pgm_read_byte(&glyphs[index - first].xAdvance));
}

// Translates UTF-8 into font indices, dropping anything the font lacks.
size_t translate(const char* utf8, uint8_t* out) {
  size_t count = 0;

  if (utf8 == nullptr) {
    return 0;
  }

  while (*utf8 != '\0' && count < MAX_GLYPHS) {
    const uint8_t index = glyphIndex(decode(utf8));

    if (index != 0) {
      out[count++] = index;
    }
  }

  return count;
}

int16_t widthOf(const TextStyle& style, const uint8_t* glyphs, size_t count) {
  if (count == 0) {
    return 0;
  }

  int16_t width = 0;

  for (size_t i = 0; i < count; i++) {
    width += advanceOf(style.font, glyphs[i]);
  }

  // Tracking sits between glyphs, not after the last one.
  return width + style.tracking * static_cast<int16_t>(count - 1);
}

void draw(Adafruit_GFX& gfx, const TextStyle& style, int16_t x,
          int16_t baseline, const uint8_t* glyphs, size_t count,
          uint16_t colour) {
  gfx.setFont(style.font);
  gfx.setTextColor(colour);
  gfx.setTextSize(1);

  for (size_t i = 0; i < count; i++) {
    gfx.setCursor(x, baseline);
    gfx.write(glyphs[i]);

    x += advanceOf(style.font, glyphs[i]) + style.tracking;
  }
}

}  // namespace

int16_t measure(const TextStyle& style, const char* utf8) {
  uint8_t glyphs[MAX_GLYPHS];

  return widthOf(style, glyphs, translate(utf8, glyphs));
}

int16_t measureGlyph(const TextStyle& style, uint32_t codepoint) {
  return advanceOf(style.font, glyphIndex(codepoint));
}

void drawLeft(Adafruit_GFX& gfx, const TextStyle& style, int16_t x,
              int16_t baseline, const char* utf8, uint16_t colour) {
  uint8_t glyphs[MAX_GLYPHS];

  draw(gfx, style, x, baseline, glyphs, translate(utf8, glyphs), colour);
}

void drawRight(Adafruit_GFX& gfx, const TextStyle& style, int16_t right,
               int16_t baseline, const char* utf8, uint16_t colour) {
  uint8_t glyphs[MAX_GLYPHS];
  const size_t count = translate(utf8, glyphs);

  draw(gfx, style, right - widthOf(style, glyphs, count), baseline, glyphs,
       count, colour);
}

void drawCentred(Adafruit_GFX& gfx, const TextStyle& style, int16_t centre,
                 int16_t baseline, const char* utf8, uint16_t colour) {
  uint8_t glyphs[MAX_GLYPHS];
  const size_t count = translate(utf8, glyphs);

  draw(gfx, style, centre - widthOf(style, glyphs, count) / 2, baseline, glyphs,
       count, colour);
}

void drawTruncated(Adafruit_GFX& gfx, const TextStyle& style, int16_t x,
                   int16_t baseline, int16_t width, const char* utf8,
                   uint16_t colour) {
  uint8_t glyphs[MAX_GLYPHS];
  size_t count = translate(utf8, glyphs);

  if (widthOf(style, glyphs, count) <= width) {
    draw(gfx, style, x, baseline, glyphs, count, colour);
    return;
  }

  const uint8_t ellipsis = glyphIndex(ELLIPSIS);
  const int16_t reserved = advanceOf(style.font, ellipsis) + style.tracking;

  // Drop glyphs from the end until the ellipsis fits too.
  while (count > 0 && widthOf(style, glyphs, count) + reserved > width) {
    count--;
  }

  glyphs[count++] = ellipsis;

  draw(gfx, style, x, baseline, glyphs, count, colour);
}

}  // namespace ui
