#include <Arduino.h>
#include <GxEPD2_BW.h>

GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> display(GxEPD2_750_T7(/*CS=*/5, /*DC=*/ 0, /*RST=*/ 2, /*BUSY=*/ 15));

void setup()
{

  display.init(115200);

  display.setTextColor(GxEPD_BLACK);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.print("Hello World!");
  }
  while (display.nextPage());

  display.hibernate();
}

void loop() {};
