package calendar

import "time"

type Event struct {
	Start       time.Time `json:"start"`
	End         time.Time `json:"end"`
	ID          string    `json:"id"`
	Title       string    `json:"title"`
	Description string    `json:"description"`
	Location    string    `json:"location"`
	AllDay      bool      `json:"all_day"`
}

// Events is a series ordered ascending by start time.
type Events []Event

// Limit returns at most n events from the front of the series.
func (e Events) Limit(n int) Events {
	if len(e) < n {
		return e
	}

	return e[:n]
}
