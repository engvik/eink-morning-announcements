package message

import (
	"errors"
	"time"
)

var (
	ErrEmptyMessage = errors.New("Empty message")
)

type Message struct {
	Time    time.Time `json:"time"`
	Message string    `json:"message"`
}

func (m Message) Valid() error {
	if m.Message == "" {
		return ErrEmptyMessage
	}

	return nil
}
