#include "icons/symbols.h"

#include <algorithm>
#include <cstddef>
#include <cstring>

#include "icons/symbols_table.h"

namespace {

constexpr size_t SYMBOL_COUNT = sizeof(SYMBOL_TABLE) / sizeof(SYMBOL_TABLE[0]);

// std::strcmp is not constexpr, so the ordering check needs its own.
constexpr int compare(const char* a, const char* b) {
  while (*a != '\0' && *a == *b) {
    a++;
    b++;
  }

  return static_cast<unsigned char>(*a) - static_cast<unsigned char>(*b);
}

constexpr bool isSorted() {
  for (size_t i = 1; i < SYMBOL_COUNT; i++) {
    if (compare(SYMBOL_TABLE[i - 1].code, SYMBOL_TABLE[i].code) >= 0) {
      return false;
    }
  }

  return true;
}

// Order is a compile-time guarantee rather than something the generator is
// trusted to have got right.
static_assert(isSorted(), "SYMBOL_TABLE must be sorted by code");

}  // namespace

const WeatherSymbol* findSymbol(const char* code) {
  if (code == nullptr || *code == '\0') {
    return nullptr;
  }

  const WeatherSymbol* end = SYMBOL_TABLE + SYMBOL_COUNT;
  const WeatherSymbol* hit =
      std::lower_bound(SYMBOL_TABLE, end, code,
                       [](const WeatherSymbol& symbol, const char* wanted) {
                         return std::strcmp(symbol.code, wanted) < 0;
                       });

  if (hit == end || std::strcmp(hit->code, code) != 0) {
    return nullptr;
  }

  return hit;
}
