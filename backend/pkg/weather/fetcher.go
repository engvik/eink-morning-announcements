package weather

import (
	"context"
	"encoding/json"
	"fmt"
	"log"
	"net/http"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
	"github.com/engvik/eink-morning-announcements/backend/internal/transport"
)

type httpClient interface {
	Request(*http.Request) ([]byte, int, error)
}

type Fetcher struct {
	Endpoint string
	HTTP     httpClient
}

func NewFetcher(http *transport.HTTP, cfg *config.Config) *Fetcher {
	// The MET API terms of service require coordinates truncated to max 4 decimals.
	weatherEndpoint := fmt.Sprintf("%s/complete?lat=%.4f&lon=%.4f", cfg.WeatherURL, cfg.WeatherLocationLat, cfg.WeatherLocationLon)

	return &Fetcher{
		Endpoint: weatherEndpoint,
		HTTP:     http,
	}
}

func (f *Fetcher) Fetch(ctx context.Context) ([]Forecast, error) {
	req, err := http.NewRequest(http.MethodGet, f.Endpoint, nil)
	if err != nil {
		return []Forecast{}, fmt.Errorf("Error creating request: %w", err)
	}

	req = req.WithContext(ctx)

	body, status, err := f.HTTP.Request(req)
	if err != nil {
		return []Forecast{}, err
	}

	// The MET API answers 203 when the requested product version is deprecated.
	// The body is still valid, so keep going, but make the notice visible.
	switch status {
	case http.StatusOK:
	case http.StatusNonAuthoritativeInfo:
		log.Printf("Weather API version is deprecated, see %s\n", f.Endpoint)
	default:
		return []Forecast{}, fmt.Errorf("Unexpected HTTP status %d", status)
	}

	var respJSON forecastResponse

	if err := json.Unmarshal(body, &respJSON); err != nil {
		return []Forecast{}, fmt.Errorf("Error unmarshaling JSON: %w", err)
	}

	forecasts := make([]Forecast, 0, len(respJSON.Properties.Timeseries))

	for _, forecast := range respJSON.Properties.Timeseries {
		forecasts = append(forecasts, Forecast{
			Time:        forecast.Time,
			Instant:     forecast.Data.Instant.Details,
			OneHour:     forecast.Data.OneHour.forecast(),
			SixHours:    forecast.Data.SixHours.forecast(),
			TwelveHours: forecast.Data.TwelveHours.forecast(),
		})
	}

	return forecasts, nil
}
