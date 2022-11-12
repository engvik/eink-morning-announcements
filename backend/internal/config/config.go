package config

import "time"

type Config struct {
	CalendarPeek            time.Duration `envconfig:"CALENDAR_PEEK" default:"168h"`
	CalendarUpdateInterval  time.Duration `envconfig:"CALENDAR_UPDATE_INTERVAL" default:"1h"`
	CalendarTimeout         time.Duration `envconfig:"CALENDAR_TIMEOUT_INTERVAL" default:"30s"`
	WeatherUpdateInterval   time.Duration `envconfig:"WEATHER_UPDATE_INTERVAL" default:"1h"`
	WeatherTimeout          time.Duration `envconfig:"WEATHER_TIMEOUT_INTERVAL" default:"30s"`
	HTTPTimeout             time.Duration `envconfig:"HTTP_TIMEOUT" default:"10s"`
	HTTPDialTimeout         time.Duration `envconfig:"HTTP_DIAL_TIMEOUT" default:"5s"`
	HTTPTLSHandshakeTimeout time.Duration `envconfig:"HTTP_TLS_HANDSHAKE_TIMEOUT" default:"5s"`
	Port                    string        `envconfig:"PORT" default:"8080"`
	CalendarURL             string        `envconfig:"CALENDAR_URL" required:"true"`
	WeatherURL              string        `envconfig:"WEATHER_URL" required:"true"`
	SQLitePath              string        `envconfig:"SQLITE_PATH" required:"true"`
	Authorization           string        `envconfig:"AUTHORIZATION"`
	WeatherLocationLat      float64       `envconfig:"WEATHER_LOCATION_LAT" default:"59.89"`
	WeatherLocationLon      float64       `envconfig:"WEATHER_LOCATION_LON" default:"10.66"`
}
