package calendar

import "time"

type Event struct {
	Start       time.Time `json:"start"`
	End         time.Time `json:"end"`
	ID          string    `json:"id"`
	Title       string    `json:"title"`
	Description string    `json:"description"`
	Location    string    `json:"location"`
}
