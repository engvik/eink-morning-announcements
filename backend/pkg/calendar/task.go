package calendar

import (
	"context"
	"log"
	"time"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
)

type fetcher interface {
	Fetch(context.Context) ([]byte, error)
}

type parser interface {
	Parse(string) ([]Event, error)
}

type store interface {
	SetCalendarEvents(context.Context, []Event) error
	GetCalendarEvents(context.Context) ([]Event, error)
}

type Task struct {
	Fetcher        fetcher
	Parser         parser
	Storage        store
	updateInterval time.Duration
	timeout        time.Duration
}

func NewTask(s store, f fetcher, p parser, cfg *config.Config) *Task {
	return &Task{
		Fetcher:        f,
		Parser:         p,
		Storage:        s,
		updateInterval: cfg.CalendarUpdateInterval,
		timeout:        cfg.CalendarTimeout,
	}
}

func (t *Task) Name() string {
	return "Calendar"
}

func (t *Task) Run() {
	ticker := time.NewTicker(t.updateInterval)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
			ctx, cancel := context.WithTimeout(context.Background(), t.timeout)

			body, err := t.Fetcher.Fetch(ctx)
			if err != nil {
				log.Printf("Error fetching calendar: %s\n", err)
				continue
			}

			events, err := t.Parser.Parse(string(body))
			if err != nil {
				log.Printf("Error parsing calendar: %s\n", err)
				continue
			}

			if err := t.Storage.SetCalendarEvents(ctx, events); err != nil {
				log.Printf("Error storing calendar events: %s\n", err)
			}

			cancel()
		}
	}
}
