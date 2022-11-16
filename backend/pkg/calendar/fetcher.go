package calendar

import (
	"context"
	"fmt"
	"net/http"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
)

type httpClient interface {
	Request(*http.Request) ([]byte, int, error)
}

type Fetcher struct {
	URL  string
	HTTP httpClient
}

func NewFetcher(http httpClient, cfg *config.Config) *Fetcher {
	return &Fetcher{
		URL:  cfg.CalendarURL,
		HTTP: http,
	}
}

func (f *Fetcher) Fetch(ctx context.Context) ([]byte, error) {
	req, err := http.NewRequest(http.MethodGet, f.URL, nil)
	if err != nil {
		return nil, fmt.Errorf("Error creating request: %w", err)
	}

	req = req.WithContext(ctx)

	body, status, err := f.HTTP.Request(req)
	if err != nil {
		return nil, err
	}

	if status != http.StatusOK {
		return nil, fmt.Errorf("Unexpected HTTP status %d:", status)
	}

	return body, nil
}
