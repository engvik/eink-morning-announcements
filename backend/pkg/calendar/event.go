package calendar

import "time"

type Event struct {
	Start       time.Time
	End         time.Time
	Title       string
	Description string
	Location    string
}
