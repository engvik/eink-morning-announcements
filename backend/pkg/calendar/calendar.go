package calendar

import (
	"context"
	"io"
	"log"
	"net/http"
	"time"

	"github.com/engvik/eink/backend/internal/config"
)

type parser interface {
	Parse(string) ([]Event, error)
}

type store interface {
	SetCalendarEvents(context.Context, []Event) error
	GetCalendarEvents(context.Context) ([]Event, error)
}

type Calendar struct {
	URL        string
	Parser     parser
	HTTPClient *http.Client
	Storage    store
}

func NewTask(c *http.Client, s store, p parser, cfg *config.Config) *Calendar {
	return &Calendar{
		URL:        cfg.CalendarURL,
		Parser:     p,
		HTTPClient: c,
		Storage:    s,
	}
}

func (c *Calendar) Name() string {
	return "Calendar"
}

func (c *Calendar) Run() {
	ticker := time.NewTicker(time.Second * 5)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
			ctx := context.Background()

			req, err := http.NewRequest(http.MethodGet, c.URL, nil)
			if err != nil {
				log.Println("Error creating request: %s\n", err)
				continue
			}

			resp, err := c.HTTPClient.Do(req)
			if err != nil {
				log.Printf("Error getting calendar: %s\n", err)
				continue
			}

			body, err := readBody(resp)
			if err != nil {
				log.Printf("Error reading calendar body: %s\n", err)
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
		}
	}
}

func readBody(resp *http.Response) ([]byte, error) {
	defer resp.Body.Close()

	return io.ReadAll(resp.Body)
}
