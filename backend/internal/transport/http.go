package transport

import (
	"fmt"
	"io"
	"net"
	"net/http"
	"time"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
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
	return "eink/1.0.0 github.com/engvik/eink-morning-announcements"
}
