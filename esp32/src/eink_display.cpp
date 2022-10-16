#include <GxEPD2_BW.h>

#include "eink_display.h"

GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> display(GxEPD2_750_T7(/*CS=*/5, /*DC=*/ 0, /*RST=*/ 2, /*BUSY=*/ 15));

void EinkDisplay::init() {
  display.init(115200);
}

void EinkDisplay::hibernate() {
  display.hibernate();
}

void EinkDisplay::refreshScreen() {
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
  }
  while (display.nextPage());
}
