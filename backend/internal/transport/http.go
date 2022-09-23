package transport

import (
	"fmt"
	"io"
	"net"
	"net/http"
	"time"

	"github.com/go-chi/cors"

	"github.com/engvik/eink/backend/internal/config"
)

type HTTP struct {
	Client *http.Client
}

func NewHTTPClient(cfg *config.Config) *HTTP {
	return &HTTP{
		Client: &http.Client{
			Timeout: time.Second * cfg.HTTPTimeout,
			Transport: &http.Transport{
				Dial: (&net.Dialer{
					Timeout: time.Second * cfg.HTTPDialTimeout,
				}).Dial,
				TLSHandshakeTimeout: time.Second * cfg.HTTPTLSHandshakeTimeout,
			},
		},
	}
}

func (http *HTTP) Request(req *http.Request) ([]byte, int, error) {
	req.Header.Set("User-Agent", http.UserAgent())

	resp, err := http.Client.Do(req)
	if err != nil {
		return nil, 0, fmt.Errorf("Error doing request: %w", err)
	}

	defer resp.Body.Close()
	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, 0, fmt.Errorf("Error reading body: %w", err)
	}

	return body, resp.StatusCode, nil
}

func (http *HTTP) UserAgent() string {
	return "eink/1.0.0 github.com/engvik/eink (will be public)"
}

func CORSHandler() func(next http.Handler) http.Handler {
	return cors.Handler(cors.Options{
		// Allow options requests to pass through to the middlewares
		AllowedOrigins: []string{"http://localhost:5173"},
		AllowedMethods: []string{
			"GET",
			"POST",
			"OPTIONS",
		},
		// Default go-chi headers + our custom header for identifying source
		AllowedHeaders: []string{
			"Origin",
			"Accept",
			"Content-Type",
			"If-None-Match",
		},
	})
}
