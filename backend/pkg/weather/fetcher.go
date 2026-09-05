package weather

import (
	"context"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"net/url"
	"time"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
	"github.com/engvik/eink-morning-announcements/backend/internal/transport"
)

type httpClient interface {
	Request(*http.Request) ([]byte, int, error)
}

type Fetcher struct {
	ForecastURL string
	SunURL      string
	Lat         float64
	Lon         float64
	Location    *time.Location
	HTTP        httpClient
}

func NewFetcher(cfg *config.Config, http *transport.HTTP) (*Fetcher, error) {
	location, err := time.LoadLocation(cfg.Location)
	if err != nil {
		return nil, err
	}

	// The MET API terms of service require coordinates truncated to max 4 decimals.
	weatherEndpoint := fmt.Sprintf("%s/complete?lat=%.4f&lon=%.4f", cfg.WeatherURL, cfg.WeatherLocationLat, cfg.WeatherLocationLon)

	return &Fetcher{
		ForecastURL: weatherEndpoint,
		SunURL:      cfg.SunURL,
		Lat:         cfg.WeatherLocationLat,
		Lon:         cfg.WeatherLocationLon,
		Location:    location,
		HTTP:        http,
	}, nil
}

// FetchSun asks for today's sunrise and sunset. The date is part of the
// request, so unlike the forecast endpoint this URL is built per call.
func (f *Fetcher) FetchSun(ctx context.Context) (Sun, error) {
	now := time.Now().In(f.Location)
	date := now.Format(time.DateOnly)

	endpoint := fmt.Sprintf(
		"%s/sun?lat=%.4f&lon=%.4f&date=%s&offset=%s",
		f.SunURL, f.Lat, f.Lon, date, url.QueryEscape(utcOffset(now)),
	)

	req, err := http.NewRequest(http.MethodGet, endpoint, nil)
	if err != nil {
		return Sun{}, fmt.Errorf("Error creating request: %w", err)
	}

	req = req.WithContext(ctx)

	body, status, err := f.HTTP.Request(req)
	if err != nil {
		return Sun{}, err
	}

	if status != http.StatusOK {
		return Sun{}, fmt.Errorf("Unexpected HTTP status %d", status)
	}

	var respJSON sunResponse

	if err := json.Unmarshal(body, &respJSON); err != nil {
		return Sun{}, fmt.Errorf("Error unmarshaling JSON: %w", err)
	}

	sunrise, err := respJSON.Properties.Sunrise.parse()
	if err != nil {
		return Sun{}, fmt.Errorf("Error parsing sunrise: %w", err)
	}

	sunset, err := respJSON.Properties.Sunset.parse()
	if err != nil {
		return Sun{}, fmt.Errorf("Error parsing sunset: %w", err)
	}

	return Sun{
		Date:    date,
		Sunrise: sunrise.In(f.Location),
		Sunset:  sunset.In(f.Location),
	}, nil
}

func (f *Fetcher) Fetch(ctx context.Context) (Forecasts, error) {
	req, err := http.NewRequest(http.MethodGet, f.ForecastURL, nil)
	if err != nil {
		return Forecasts{}, fmt.Errorf("Error creating request: %w", err)
	}

	req = req.WithContext(ctx)

	body, status, err := f.HTTP.Request(req)
	if err != nil {
		return Forecasts{}, err
	}

	// The MET API answers 203 when the requested product version is deprecated.
	// The body is still valid, so keep going, but make the notice visible.
	switch status {
	case http.StatusOK:
	case http.StatusNonAuthoritativeInfo:
		log.Printf("Weather API version is deprecated, see %s\n", f.ForecastURL)
	default:
		return Forecasts{}, fmt.Errorf("Unexpected HTTP status %d", status)
	}

	var respJSON forecastResponse

	if err := json.Unmarshal(body, &respJSON); err != nil {
		return Forecasts{}, fmt.Errorf("Error unmarshaling JSON: %w", err)
	}

	forecasts := make(Forecasts, 0, len(respJSON.Properties.Timeseries))

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
