package news

import (
	"context"
	"fmt"
	"net/http"
)

type httpClient interface {
	Request(*http.Request) ([]byte, int, error)
}

type Fetcher struct {
	HTTP httpClient
}

func NewFetcher(http httpClient) *Fetcher {
	return &Fetcher{
		HTTP: http,
	}
}

// Fetch retrieves the raw feed at url.
func (f *Fetcher) Fetch(ctx context.Context, url string) ([]byte, error) {
	req, err := http.NewRequest(http.MethodGet, url, nil)
	if err != nil {
		return nil, fmt.Errorf("Error creating request: %w", err)
	}

	req = req.WithContext(ctx)

	body, status, err := f.HTTP.Request(req)
	if err != nil {
		return nil, err
	}

	if status != http.StatusOK {
		return nil, fmt.Errorf("Unexpected HTTP status %d", status)
	}

	return body, nil
}
