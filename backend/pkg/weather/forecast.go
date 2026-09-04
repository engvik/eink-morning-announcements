package weather

import "time"

type Forecast struct {
	Time        time.Time       `json:"time"`
	Instant     InstantForecast `json:"instant"`
	OneHour     PeriodForecast  `json:"one_hour"`
	SixHours    PeriodForecast  `json:"six_hours"`
	TwelveHours PeriodForecast  `json:"twelve_hours"`
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
