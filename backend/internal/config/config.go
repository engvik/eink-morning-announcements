package config

import (
	"fmt"
	"strings"
	"time"
)

type Config struct {
	CalendarPeek            time.Duration `envconfig:"CALENDAR_PEEK" default:"168h"`
	CalendarUpdateInterval  time.Duration `envconfig:"CALENDAR_UPDATE_INTERVAL" default:"1h"`
	CalendarTimeout         time.Duration `envconfig:"CALENDAR_TIMEOUT_INTERVAL" default:"30s"`
	WeatherUpdateInterval   time.Duration `envconfig:"WEATHER_UPDATE_INTERVAL" default:"1h"`
	WeatherTimeout          time.Duration `envconfig:"WEATHER_TIMEOUT_INTERVAL" default:"30s"`
	NewsUpdateInterval      time.Duration `envconfig:"NEWS_UPDATE_INTERVAL" default:"15m"`
	NewsTimeout             time.Duration `envconfig:"NEWS_TIMEOUT_INTERVAL" default:"30s"`
	MyKidTimeout            time.Duration `envconfig:"MYKID_TIMEOUT_INTERVAL" default:"30s"`
	HTTPTimeout             time.Duration `envconfig:"HTTP_TIMEOUT" default:"10s"`
	HTTPDialTimeout         time.Duration `envconfig:"HTTP_DIAL_TIMEOUT" default:"5s"`
	HTTPTLSHandshakeTimeout time.Duration `envconfig:"HTTP_TLS_HANDSHAKE_TIMEOUT" default:"5s"`
	Port                    string        `envconfig:"PORT" default:"8080"`
	WeatherURL              string        `envconfig:"WEATHER_URL" required:"true"`
	SunURL                  string        `envconfig:"SUN_URL" required:"true"`
	SQLitePath              string        `envconfig:"SQLITE_PATH" required:"true"`
	Location                string        `envconfig:"LOCATION" default:"Europe/Oslo"`
	Authorization           string        `envconfig:"AUTHORIZATION"`
	NewsFeeds               Feeds         `envconfig:"NEWS_FEEDS"`
	CalendarURLs            []string      `envconfig:"CALENDAR_URL" required:"true"`
	CalendarExclude         []string      `envconfig:"CALENDAR_EXCLUDE"`
	MyKidPhone              string        `envconfig:"MYKID_PHONE"`
	MyKidPassword           string        `envconfig:"MYKID_PASSWORD"`
	MyKidKids               []string      `envconfig:"MYKID_KIDS"`
	MyKidUpdateTimes        Clocks        `envconfig:"MYKID_UPDATE_TIMES" default:"07:25,15:00"`
	WeatherLocationLat      float64       `envconfig:"WEATHER_LOCATION_LAT" default:"59.89"`
	WeatherLocationLon      float64       `envconfig:"WEATHER_LOCATION_LON" default:"10.66"`
	CalendarFetchEvents     int           `envconfig:"CALENDAR_FETCH_EVENTS" default:"100"`
	WeatherFetchEorecasts   int           `envconfig:"WEATHER_FETCH_FORECASTS" default:"24"`
	NewsFetchItems          int           `envconfig:"NEWS_FETCH_ITEMS" default:"10"`
}

// Feed is a news feed and the short label the display shows as its source.
type Feed struct {
	Label string
	URL   string
}

// Feeds decodes comma separated LABEL=URL pairs, keeping their order.
type Feeds []Feed

func (f *Feeds) Decode(value string) error {
	for entry := range strings.SplitSeq(value, ",") {
		entry = strings.TrimSpace(entry)
		if entry == "" {
			continue
		}

		// Cut at the first =, so a query string in the URL survives.
		label, url, ok := strings.Cut(entry, "=")
		if !ok || label == "" || url == "" || strings.Contains(label, "/") {
			return fmt.Errorf("invalid feed %q, want LABEL=URL", entry)
		}

		*f = append(*f, Feed{Label: label, URL: url})
	}

	return nil
}

// Clock is a time of day.
type Clock struct {
	Hour, Minute int
}

// Clocks decodes comma separated HH:MM times of day.
type Clocks []Clock

func (c *Clocks) Decode(value string) error {
	*c = nil

	for entry := range strings.SplitSeq(value, ",") {
		entry = strings.TrimSpace(entry)
		if entry == "" {
			continue
		}

		t, err := time.Parse("15:04", entry)
		if err != nil {
			return fmt.Errorf("invalid time %q, want HH:MM", entry)
		}

		*c = append(*c, Clock{Hour: t.Hour(), Minute: t.Minute()})
	}

	if len(*c) == 0 {
		return fmt.Errorf("no times given")
	}

	return nil
}

// Next returns the first of the times strictly after now, in now's location.
func (c Clocks) Next(now time.Time) time.Time {
	var next time.Time

	for _, clock := range c {
		t := time.Date(now.Year(), now.Month(), now.Day(), clock.Hour, clock.Minute, 0, 0, now.Location())
		if !t.After(now) {
			t = time.Date(now.Year(), now.Month(), now.Day()+1, clock.Hour, clock.Minute, 0, 0, now.Location())
		}

		if next.IsZero() || t.Before(next) {
			next = t
		}
	}

	return next
}
