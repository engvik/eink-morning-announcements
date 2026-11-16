package transport

import (
	"net/http"
	"strings"
	"testing"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
)

// A failed request must not put the URL's secret query params into the error.
func TestRequestErrorHidesQuery(t *testing.T) {
	h := NewHTTPClient(&config.Config{})

	req, err := http.NewRequest(http.MethodGet, "https://127.0.0.1:1/calendar?uid=SECRET_TOKEN", nil)
	if err != nil {
		t.Fatal(err)
	}

	_, _, err = h.Request(req)
	if err == nil {
		t.Fatal("expected a connection error")
	}

	if strings.Contains(err.Error(), "SECRET_TOKEN") {
		t.Errorf("error leaks query secret: %v", err)
	}

	if !strings.Contains(err.Error(), "127.0.0.1:1") {
		t.Errorf("error should name the host for debugging: %v", err)
	}
}
