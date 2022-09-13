package transport

import (
	"fmt"
	"io"
	"net"
	"net/http"
	"time"
)

type HTTP struct {
	Client *http.Client
}

func NewHTTPClient() *HTTP {
	return &HTTP{
		Client: &http.Client{
			Timeout: time.Second * 10,
			Transport: &http.Transport{
				Dial: (&net.Dialer{
					Timeout: 5 * time.Second,
				}).Dial,
				TLSHandshakeTimeout: 5 * time.Second,
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
