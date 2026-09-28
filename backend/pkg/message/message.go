package message

import (
	"time"
)

type Message struct {
	Time    time.Time `json:"time"`
	Message string    `json:"message"`
}
