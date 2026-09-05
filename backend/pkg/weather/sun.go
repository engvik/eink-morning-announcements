package weather

import (
	"fmt"
	"time"
)

// sunTimeLayout is what the Sunrise API emits. It is RFC3339 without seconds,
// so it will not unmarshal straight into a time.Time.
const sunTimeLayout = "2006-01-02T15:04Z07:00"

type Sun struct {
	Date    string    `json:"date"`
	Sunrise time.Time `json:"sunrise"`
	Sunset  time.Time `json:"sunset"`
}

type sunResponse struct {
	Properties struct {
		Sunrise sunEvent `json:"sunrise"`
		Sunset  sunEvent `json:"sunset"`
	} `json:"properties"`
}

type sunEvent struct {
	Time string `json:"time"`
}

// parse returns the zero time when the event is absent, which is how the API
// reports midnight sun and polar night.
func (e sunEvent) parse() (time.Time, error) {
	if e.Time == "" {
		return time.Time{}, nil
	}

	return time.Parse(sunTimeLayout, e.Time)
}

// utcOffset renders the location's current offset as the API wants it, +02:00.
func utcOffset(t time.Time) string {
	_, seconds := t.Zone()

	sign := "+"
	if seconds < 0 {
		sign = "-"
		seconds = -seconds
	}

	return fmt.Sprintf("%s%02d:%02d", sign, seconds/3600, (seconds%3600)/60)
}
