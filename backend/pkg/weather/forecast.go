package weather

import "time"

type Forecast struct {
	Time        time.Time       `json:"time"`
	Instant     InstantForecast `json:"instant"`
	OneHour     PeriodForecast  `json:"one_hour"`
	SixHours    PeriodForecast  `json:"six_hours"`
	TwelveHours PeriodForecast  `json:"twelve_hours"`
}

type InstantForecast struct {
	AirPressureAtSeaLevel float64 `json:"air_pressure_at_sea_level"`
	AirTemperature        float64 `json:"air_temperature"`
	CloudAreaFraction     float64 `json:"cloud_area_fraction"`
	RelativeHumidity      float64 `json:"relative_humidity"`
	WindFromDirection     float64 `json:"wind_from_direction"`
	WindSpeed             float64 `json:"wind_speed"`
}

type PeriodForecast struct {
	SymbolCode          string  `json:"symbol_code"`
	PrecipitationAmount float64 `json:"precipitation_amount"`
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
		PrecipitationAmount float64 `json:"precipitation_amount"`
	} `json:"details"`
}
