package transport

import (
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

func (http *HTTP) UserAgent() string {
	return "eink/1.0.0 github.com/engvik/eink (will be public)"
}
