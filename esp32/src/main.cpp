#include <Arduino.h>
#include <GxEPD2_BW.h>

GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT/2> display(GxEPD2_750_T7(/*CS=*/15, /*DC=*/ 27, /*RST=*/ 26, /*BUSY=*/ 25));

void setup() {
  display.init();
  display.setTextColor(GxEPD_BLACK);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.print("Hello boiiiiii");
  }
  while (display.nextPage());
}

void loop() {}