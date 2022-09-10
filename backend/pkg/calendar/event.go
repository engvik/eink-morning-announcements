package calendar

import "time"

type Event struct {
	Start       time.Time
	End         time.Time
	ID          string
	Title       string
	Description string
	Location    string
}
