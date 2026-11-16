package message

import (
	"time"
)

// Sources a message can come from.
const (
	SourceManual = "manual"
	SourceMyKid  = "mykid"
)

type Message struct {
	Time    time.Time `json:"time"`
	Message string    `json:"message"`
	Source  string    `json:"source"`

	// ExternalID identifies the upstream item, so it is stored only once.
	ExternalID string `json:"-"`
}
