package config

import "time"

type Config struct {
	Port         string        `envconfig:"PORT" default:"8080"`
	CalendarURL  string        `envconfig:"CALENDAR_URL" required:"true"`
	CalendarPeek time.Duration `envconfig:"CALENDAR_PEEK" default:"168h"`
}
