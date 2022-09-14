package calendar

import (
	"context"
	"log"
	"time"

	"github.com/engvik/eink/backend/internal/config"
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

type Calendar struct {
	Fetcher        fetcher
	Parser         parser
	Storage        store
	updateInterval time.Duration
	timeout        time.Duration
}

func NewTask(s store, f fetcher, p parser, cfg *config.Config) *Calendar {
	return &Calendar{
		Fetcher:        f,
		Parser:         p,
		Storage:        s,
		updateInterval: cfg.CalendarUpdateInterval,
		timeout:        cfg.CalendarTimeout,
	}
}

func (c *Calendar) Name() string {
	return "Calendar"
}

func (c *Calendar) Run() {
	ticker := time.NewTicker(c.updateInterval)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
			ctx, cancel := context.WithTimeout(context.Background(), c.timeout)

			body, err := c.Fetcher.Fetch(ctx)
			if err != nil {
				log.Printf("Error fetching calendar: %s\n", err)
				continue
			}

			events, err := c.Parser.Parse(string(body))
			if err != nil {
				log.Printf("Error parsing calendar: %s\n", err)
				continue
			}

			if err := c.Storage.SetCalendarEvents(ctx, events); err != nil {
				log.Printf("Error storing calendar events: %s\n", err)
			}

			cancel()
		}
	}
}
