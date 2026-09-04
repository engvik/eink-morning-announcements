package weather

import "time"

type Forecast struct {
	Time        time.Time       `json:"time"`
	Instant     InstantForecast `json:"instant"`
	OneHour     PeriodForecast  `json:"one_hour"`
	SixHours    PeriodForecast  `json:"six_hours"`
	TwelveHours PeriodForecast  `json:"twelve_hours"`
}

// Forecasts is a series ordered ascending by time.
type Forecasts []Forecast

// Limit returns at most n forecasts from the front of the series.
func (f Forecasts) Limit(n int) Forecasts {
	if len(f) < n {
		return f
	}

	return f[:n]
}

// Summarize groups the series by calendar date. The forecasts must already be
// converted to the display location. The first day is today, and covers only
// the hours remaining in it.
//
// MET switches from hourly to six-hourly entries a couple of days out, so
// precipitation is summed by walking forward and taking whichever period block
// is present, skipping entries an earlier longer block already covered. A
// six-hour block straddling midnight counts towards the day it starts in.
func (f Forecasts) Summarize() []Day {
	days := make([]Day, 0, 10)
	index := make(map[string]int, 10)

	var coveredUntil time.Time

	for _, forecast := range f {
		date := forecast.Time.Format(time.DateOnly)

		i, ok := index[date]
		if !ok {
			days = append(days, Day{
				Date:              date,
				AirTemperatureMin: forecast.Instant.AirTemperature,
				AirTemperatureMax: forecast.Instant.AirTemperature,
			})

			i = len(days) - 1
			index[date] = i
		}

		day := &days[i]

		day.AirTemperatureMin = min(day.AirTemperatureMin, forecast.Instant.AirTemperature)
		day.AirTemperatureMax = max(day.AirTemperatureMax, forecast.Instant.AirTemperature)
		day.UltravioletIndexMax = max(day.UltravioletIndexMax, forecast.Instant.UltravioletIndexClearSky)

		if forecast.Time.Before(coveredUntil) {
			continue
		}

		switch {
		case forecast.OneHour.SymbolCode != "":
			day.PrecipitationAmount += forecast.OneHour.PrecipitationAmount
			coveredUntil = forecast.Time.Add(time.Hour)
		case forecast.SixHours.SymbolCode != "":
			day.PrecipitationAmount += forecast.SixHours.PrecipitationAmount
			coveredUntil = forecast.Time.Add(6 * time.Hour)
		}
	}

	return days
}

// Day aggregates every forecast falling on one calendar date.
type Day struct {
	Date                string  `json:"date"`
	AirTemperatureMin   float64 `json:"air_temperature_min"`
	AirTemperatureMax   float64 `json:"air_temperature_max"`
	PrecipitationAmount float64 `json:"precipitation_amount"`
	UltravioletIndexMax float64 `json:"ultraviolet_index_max"`
}

// InstantForecast describes conditions at the forecast timestamp itself.
type InstantForecast struct {
	AirPressureAtSeaLevel    float64 `json:"air_pressure_at_sea_level"`
	AirTemperature           float64 `json:"air_temperature"`
	ApparentAirTemperature   float64 `json:"apparent_air_temperature"`
	CloudAreaFraction        float64 `json:"cloud_area_fraction"`
	RelativeHumidity         float64 `json:"relative_humidity"`
	UltravioletIndexClearSky float64 `json:"ultraviolet_index_clear_sky"`
	WindFromDirection        float64 `json:"wind_from_direction"`
	WindSpeed                float64 `json:"wind_speed"`
	WindSpeedOfGust          float64 `json:"wind_speed_of_gust"`
}

// PeriodForecast covers next_1_hours, next_6_hours and next_12_hours. MET does
// not supply every field for every period: min/max temperatures come only with
// next_6_hours, and next_12_hours carries no precipitation amount. Absent
// fields are zero.
type PeriodForecast struct {
	SymbolCode                 string  `json:"symbol_code"`
	AirTemperatureMax          float64 `json:"air_temperature_max"`
	AirTemperatureMin          float64 `json:"air_temperature_min"`
	PrecipitationAmount        float64 `json:"precipitation_amount"`
	ProbabilityOfPrecipitation float64 `json:"probability_of_precipitation"`
}

type forecastResponse struct {
	Properties struct {
		Timeseries []struct {
			Time time.Time `json:"time"`
			Data struct {
				Instant struct {
					Details InstantForecast `json:"details"`
				} `json:"instant"`
				OneHour     periodForecastResponse `json:"next_1_hours"`
				SixHours    periodForecastResponse `json:"next_6_hours"`
				TwelveHours periodForecastResponse `json:"next_12_hours"`
			} `json:"data"`
		} `json:"timeseries"`
	} `json:"properties"`
}

type periodForecastResponse struct {
	Summary struct {
		SymbolCode string `json:"symbol_code"`
	} `json:"summary"`
	Details struct {
		AirTemperatureMax          float64 `json:"air_temperature_max"`
		AirTemperatureMin          float64 `json:"air_temperature_min"`
		PrecipitationAmount        float64 `json:"precipitation_amount"`
		ProbabilityOfPrecipitation float64 `json:"probability_of_precipitation"`
	} `json:"details"`
}

func (p periodForecastResponse) forecast() PeriodForecast {
	return PeriodForecast{
		SymbolCode:                 p.Summary.SymbolCode,
		AirTemperatureMax:          p.Details.AirTemperatureMax,
		AirTemperatureMin:          p.Details.AirTemperatureMin,
		PrecipitationAmount:        p.Details.PrecipitationAmount,
		ProbabilityOfPrecipitation: p.Details.ProbabilityOfPrecipitation,
	}
}
