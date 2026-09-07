// Renders the display to a PNG on the host, so layout work does not need a
// flash cycle. The canvas is the same size as the panel and drawing goes
// through Adafruit_GFX, exactly as it does on the device.
#include <Adafruit_GFX.h>

#include <cstdio>
#include <string>

#include "fonts/ArchivoBlack56.h"
#include "fonts/ArchivoBold19.h"
#include "fonts/ArchivoSemiBold16.h"
#include "fonts/MonoRegular13.h"
#include "fonts/MonoSemiBold15.h"
#include "png.h"

namespace {

// The panel is 800x480 rotated to portrait.
constexpr int16_t kWidth = 480;
constexpr int16_t kHeight = 800;

// A set bit is ink. On the device this maps to GxEPD_BLACK.
constexpr uint16_t kInk = 1;
constexpr uint16_t kPaper = 0;

// The generated fonts place these just above ASCII.
constexpr char kAe = 0x7F, kOe = 0x80, kAa = 0x81;
constexpr char kAeUpper = 0x82, kOeUpper = 0x83, kAaUpper = 0x84;
constexpr char kMiddot = 0x85, kEmDash = 0x86, kEnDash = 0x87;
constexpr char kDegree = 0x88, kEllipsis = 0x89;

void note(GFXcanvas1 &canvas, int16_t y, const char *text) {
  canvas.setFont(&MonoRegular13);
  canvas.setCursor(20, y);
  canvas.print(text);
}

// Ink height of a string, for comparing a face against the comp.
int16_t inkHeight(GFXcanvas1 &canvas, const GFXfont *font, const char *text) {
  int16_t x1, y1;
  uint16_t w, h;

  canvas.setFont(font);
  canvas.getTextBounds(text, 0, 100, &x1, &y1, &w, &h);

  return static_cast<int16_t>(h);
}

}  // namespace

int main(int argc, char **argv) {
  const std::string out = argc > 1 ? argv[1] : "preview.png";

  GFXcanvas1 canvas(kWidth, kHeight);
  canvas.fillScreen(kPaper);
  canvas.setTextColor(kInk);

  note(canvas, 30, "SPECIMEN - FIVE FACES");
  canvas.drawFastHLine(20, 40, kWidth - 40, kInk);

  // F1: hero numerals, digits and degree only.
  canvas.setFont(&ArchivoBlack56);
  canvas.setCursor(20, 110);
  canvas.print("08");
  canvas.print(kDegree);

  note(canvas, 132, "F1 ArchivoBlack56");

  // F2: primary titles.
  canvas.setFont(&ArchivoBold19);
  canvas.setCursor(20, 178);
  canvas.print("The quick brown fox 0123");
  canvas.setCursor(20, 202);
  canvas.print(kAe);
  canvas.print(kOe);
  canvas.print(kAa);
  canvas.print(" ");
  canvas.print(kAeUpper);
  canvas.print(kOeUpper);
  canvas.print(kAaUpper);

  note(canvas, 224, "F2 ArchivoBold19");

  // F3: secondary titles.
  canvas.setFont(&ArchivoSemiBold16);
  canvas.setCursor(20, 264);
  canvas.print("The quick brown fox ");
  canvas.print(kAe);
  canvas.print(kOe);
  canvas.print(kAa);

  note(canvas, 286, "F3 ArchivoSemiBold16");

  // F4: times and day keys.
  canvas.setFont(&MonoSemiBold15);
  canvas.setCursor(20, 326);
  canvas.print("00:00-00:00  ABC 00");

  note(canvas, 348, "F4 MonoSemiBold15");

  // F5: labels, meta and every symbol in the set.
  canvas.setFont(&MonoRegular13);
  canvas.setCursor(20, 388);
  canvas.print("ABCDEFGHIJKLM abcdefghijklm 0123456789");
  canvas.setCursor(20, 408);
  canvas.print("!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~");
  canvas.setCursor(20, 428);
  canvas.print(kAe);
  canvas.print(kOe);
  canvas.print(kAa);
  canvas.print(" ");
  canvas.print(kAeUpper);
  canvas.print(kOeUpper);
  canvas.print(kAaUpper);
  canvas.print("  ");
  canvas.print(kMiddot);
  canvas.print(" ");
  canvas.print(kEmDash);
  canvas.print(" ");
  canvas.print(kEnDash);
  canvas.print(" ");
  canvas.print(kDegree);
  canvas.print(" ");
  canvas.print(kEllipsis);

  note(canvas, 450, "F5 MonoRegular13 - full set");

  canvas.drawFastHLine(20, 470, kWidth - 40, kInk);

  char line[96];
  snprintf(line, sizeof(line), "ink height  F1 %d  F2 %d  F4 %d",
           inkHeight(canvas, &ArchivoBlack56, "08"),
           inkHeight(canvas, &ArchivoBold19, "Hx"),
           inkHeight(canvas, &MonoSemiBold15, "00"));
  note(canvas, 495, line);

  // 20px grid, the agenda row rhythm.
  note(canvas, 525, "20px row grid");
  for (int16_t y = 540; y < 780; y += 20) {
    canvas.drawFastHLine(20, y, 8, kInk);
  }

  if (!png::write(out, canvas.getBuffer(), kWidth, kHeight)) {
    fprintf(stderr, "could not write %s\n", out.c_str());
    return 1;
  }

  printf("wrote %s (%dx%d)\n", out.c_str(), kWidth, kHeight);

  return 0;
}
