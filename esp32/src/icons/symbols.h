#pragma once

// Lookup from a MET symbol_code to what the display needs to draw for it: the
// two icon sizes and a condition name.
//
// The table itself is generated into symbols_table.h by tools/genicons.py, so
// every code maps to the bitmap bearing its own name.
struct WeatherSymbol {
  const char* code;
  const unsigned char* icon24;
  const unsigned char* icon44;
  const char* label;
};

// Returns nullptr for an unknown or empty code.
const WeatherSymbol* findSymbol(const char* code);
