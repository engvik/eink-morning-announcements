// Just enough of the Arduino API to compile Adafruit_GFX on a host, so the
// display code can be rendered to a file without flashing.
#pragma once

#include <math.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

#include "Print.h"

#define radians(deg) ((deg) * M_PI / 180.0)

#define PROGMEM
#define PGM_P const char *

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif
// pgm_read_pointer is left to Adafruit_GFX.cpp, which defines it unguarded.

// The host has no cooperative scheduler to yield to.
inline void yield() {}

class __FlashStringHelper;

#define F(string_literal) (string_literal)

// Adafruit_GFX only ever asks a String for its length and its bytes.
class String {
 public:
  String() = default;
  String(const char *s) : value(s ? s : "") {}
  String(const std::string &s) : value(s) {}

  unsigned int length() const { return static_cast<unsigned int>(value.size()); }
  const char *c_str() const { return value.c_str(); }
  char operator[](unsigned int i) const { return value[i]; }

 private:
  std::string value;
};
