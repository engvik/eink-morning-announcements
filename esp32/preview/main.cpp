// Renders the display to a PNG on the host, so layout work does not need a
// flash cycle. The canvas is the same size as the panel and drawing goes
// through Adafruit_GFX, exactly as it does on the device.
#include <Adafruit_GFX.h>

#include <cstdio>
#include <cstring>
#include <string>

#include "png.h"
#include "ui/decode.h"
#include "ui/panel.h"
#include "ui/text.h"
#include "ui/theme.h"

namespace {

// Stand-in data with the shape the backend produces. Deliberately generic.
constexpr const char *META = R"({
  "today": "Wednesday", "month": "September", "date": 26, "week": 35,
  "now": "2026-09-26T06:00:00+02:00",
  "sunrise": "2026-09-26T05:52:00+02:00",
  "sunset": "2026-09-26T20:41:00+02:00"})";

constexpr const char *WEATHER = R"({
  "forecasts": [
    {"time": "2026-09-26T08:00:00+02:00",
     "instant": {"air_temperature": 16.4, "apparent_air_temperature": 19.2,
                 "wind_speed": 4.1, "wind_speed_of_gust": 8.7},
     "one_hour": {"symbol_code": "clearsky_day", "precipitation_amount": 0,
                  "probability_of_precipitation": 5}},
    {"time": "2026-09-26T09:00:00+02:00", "instant": {"air_temperature": 17.0},
     "one_hour": {"symbol_code": "fair_day"}},
    {"time": "2026-09-26T10:00:00+02:00", "instant": {"air_temperature": 18.8},
     "one_hour": {"symbol_code": "fair_day", "precipitation_amount": 0,
                  "probability_of_precipitation": 10}},
    {"time": "2026-09-26T11:00:00+02:00", "instant": {"air_temperature": 19.0},
     "one_hour": {"symbol_code": "cloudy"}},
    {"time": "2026-09-26T12:00:00+02:00", "instant": {"air_temperature": 21.2},
     "one_hour": {"symbol_code": "partlycloudy_day",
                  "precipitation_amount": 0.14,
                  "probability_of_precipitation": 30}},
    {"time": "2026-09-26T13:00:00+02:00", "instant": {"air_temperature": 20.0},
     "one_hour": {"symbol_code": "rain", "precipitation_amount": 1.0}},
    {"time": "2026-09-26T14:00:00+02:00", "instant": {"air_temperature": 20.4},
     "one_hour": {"symbol_code": "rain", "precipitation_amount": 1.75,
                  "probability_of_precipitation": 90}},
    {"time": "2026-09-26T15:00:00+02:00", "instant": {"air_temperature": 19.0},
     "one_hour": {"symbol_code": "lightrain", "precipitation_amount": 0.5}},
    {"time": "2026-09-26T16:00:00+02:00", "instant": {"air_temperature": 18.1},
     "one_hour": {"symbol_code": "lightrain", "precipitation_amount": 0.7,
                  "probability_of_precipitation": 70}}],
  "days": [
    {"date": "2026-09-26", "air_temperature_min": 12.4,
     "air_temperature_max": 21.0, "ultraviolet_index_max": 3.2,
     "precipitation_amount": 2.7},
    {"date": "2026-09-27", "air_temperature_max": 14.0,
     "precipitation_amount": 6.0},
    {"date": "2026-09-28", "air_temperature_max": 17.0,
     "precipitation_amount": 0.0},
    {"date": "2026-09-29", "air_temperature_max": 19.0,
     "precipitation_amount": 0.0},
    {"date": "2026-09-30", "air_temperature_max": 18.0,
     "precipitation_amount": 1.1},
    {"date": "2026-10-01", "air_temperature_max": 16.0,
     "precipitation_amount": 0.4},
    {"date": "2026-10-02", "air_temperature_max": 17.0,
     "precipitation_amount": 0.0},
    {"date": "2026-10-03", "air_temperature_max": 15.0,
     "precipitation_amount": 2.2}]})";

// Nine today, two spanning, and a week of upcoming days.
constexpr const char *CALENDAR = R"({"total": 9, "events": [
  {"start":"2026-09-25T00:00:00+02:00","end":"2026-09-29T00:00:00+02:00",
   "title":"First long span","all_day":true},
  {"start":"2026-09-27T00:00:00+02:00","end":"2026-09-30T00:00:00+02:00",
   "title":"Second long span","all_day":true},
  {"start":"2026-09-26T08:15:00+02:00","end":"2026-09-26T08:45:00+02:00","title":"Alpha"},
  {"start":"2026-09-26T09:00:00+02:00","end":"2026-09-26T09:30:00+02:00","title":"Bravo"},
  {"start":"2026-09-26T10:00:00+02:00","end":"2026-09-26T10:30:00+02:00","title":"Charlie"},
  {"start":"2026-09-26T11:30:00+02:00","end":"2026-09-26T12:00:00+02:00","title":"Delta"},
  {"start":"2026-09-26T13:00:00+02:00","end":"2026-09-26T14:30:00+02:00",
   "title":"Echo","location":"A place"},
  {"start":"2026-09-26T15:00:00+02:00","end":"2026-09-26T15:30:00+02:00","title":"Foxtrot"},
  {"start":"2026-09-26T16:30:00+02:00","end":"2026-09-26T17:00:00+02:00","title":"Golf"},
  {"start":"2026-09-26T18:00:00+02:00","end":"2026-09-26T19:00:00+02:00","title":"Hotel"},
  {"start":"2026-09-26T19:00:00+02:00","end":"2026-09-26T20:00:00+02:00",
   "title":"India, with a title long enough to need clipping in its own column"},
  {"start":"2026-09-27T17:30:00+02:00","end":"2026-09-27T18:30:00+02:00","title":"Juliett"},
  {"start":"2026-09-28T08:00:00+02:00","end":"2026-09-28T09:00:00+02:00","title":"Kilo"},
  {"start":"2026-09-29T11:00:00+02:00","end":"2026-09-29T12:00:00+02:00","title":"Lima"},
  {"start":"2026-09-30T14:00:00+02:00","end":"2026-09-30T15:00:00+02:00","title":"Mike"},
  {"start":"2026-10-01T08:15:00+02:00","end":"2026-10-01T09:00:00+02:00","title":"November"},
  {"start":"2026-10-02T17:00:00+02:00","end":"2026-10-02T18:00:00+02:00","title":"Oscar"},
  {"start":"2026-10-03T09:00:00+02:00","end":"2026-10-03T10:00:00+02:00","title":"Papa"}]})";

int render(const char *path, const ui::DisplayModel &model,
           const char *caption) {
  GFXcanvas1 canvas(ui::PANEL_WIDTH, ui::PANEL_HEIGHT);
  canvas.fillScreen(ui::PAPER);

  int16_t y = ui::drawHeader(canvas, model, ui::PADDING);
  y = ui::drawReminder(canvas, model, y);
  y = ui::drawWeather(canvas, model, y);
  y = ui::drawHourly(canvas, model, y);

  const int16_t agendaTop = y + ui::AGENDA_GAP;

  ui::drawAgenda(canvas, model, y);
  ui::drawFooter(canvas, model);

  const int16_t agendaBottom = ui::FOOTER_TOP - ui::AGENDA_GAP;

  printf("%-16s agenda %3d..%d = %3dpx\n", caption, agendaTop, agendaBottom,
         agendaBottom - agendaTop);

  return png::write(path, canvas.getBuffer(), ui::PANEL_WIDTH,
                    ui::PANEL_HEIGHT)
             ? 0
             : 1;
}

}  // namespace

int main() {
  ui::DisplayModel model;
  ui::clear(model);
  ui::decodeMeta(model, META);
  ui::decodeWeather(model, WEATHER);
  ui::decodeCalendar(model, CALENDAR);
  ui::decodeMessage(model, R"({"message":"Bins out before 07:00"})");

  // Set on the device from config and the battery reading.
  std::strncpy(model.location, "OSLO", sizeof(model.location) - 1);
  model.battery = 90;

  // 4C: events running across several days.
  if (render("agenda-running.png", model, "4C running") != 0) {
    return 1;
  }

  // 4D: nothing spanning, so AHEAD gets that space back.
  ui::DisplayModel plain = model;
  plain.runningCount = 0;

  if (render("agenda-plain.png", plain, "4D no running") != 0) {
    return 1;
  }

  // 4E: no reminder either, which hands the agenda another 44px.
  ui::DisplayModel quiet = plain;
  quiet.reminder[0] = '\0';

  if (render("agenda-quiet.png", quiet, "4E no reminder") != 0) {
    return 1;
  }

  // Nothing on today: the whole TODAY section goes, and AHEAD takes the space.
  ui::DisplayModel free_day;
  ui::clear(free_day);
  ui::decodeMeta(free_day, META);
  ui::decodeWeather(free_day, WEATHER);
  ui::decodeMessage(free_day, R"({"message":"Bins out before 07:00"})");
  ui::decodeCalendar(free_day, R"({"total": 5, "events": [
    {"start":"2026-09-27T17:30:00+02:00","end":"2026-09-27T18:30:00+02:00","title":"Juliett"},
    {"start":"2026-09-28T08:00:00+02:00","end":"2026-09-28T09:00:00+02:00","title":"Kilo"},
    {"start":"2026-09-29T11:00:00+02:00","end":"2026-09-29T12:00:00+02:00","title":"Lima"},
    {"start":"2026-09-30T14:00:00+02:00","end":"2026-09-30T15:00:00+02:00","title":"Mike"},
    {"start":"2026-10-01T08:15:00+02:00","end":"2026-10-01T09:00:00+02:00","title":"November"}]})");
  std::strncpy(free_day.location, "OSLO", sizeof(free_day.location) - 1);
  free_day.battery = 90;

  printf("free day: todayCount=%d todayTotal=%d aheadCount=%d\n",
         free_day.todayCount, free_day.todayTotal, free_day.aheadCount);

  return render("agenda-free-day.png", free_day, "free day");
}
