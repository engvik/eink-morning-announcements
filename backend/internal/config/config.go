package config

import "time"

type Config struct {
	CalendarPeek       time.Duration `envconfig:"CALENDAR_PEEK" default:"168h"`
	Port               string        `envconfig:"PORT" default:"8080"`
	CalendarURL        string        `envconfig:"CALENDAR_URL" required:"true"`
	WeatherURL         string        `envconfig:"WEATHER_URL" required:"true"`
	SQLitePath         string        `envconfig:"SQLITE_PATH" required:"true"`
	WeatherLocationLat float64       `envconfig:"WEATHER_LOCATION_LAT" default:"59.89"`
	WeatherLocationLon float64       `envconfig:"WEATHER_LOCATION_LON" default:"10.66"`
}
