package config

type Config struct {
	Port        string `envconfig:"PORT" default:"8080"`
	CalendarURL string `envconfig:"CALENDAR_URL" required:"true"`
}
