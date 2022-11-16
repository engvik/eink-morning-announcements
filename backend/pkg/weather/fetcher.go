package weather

import (
	"context"
	"encoding/json"
	"fmt"
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
	weatherEndpoint := fmt.Sprintf("%s/compact?lat=%f&lon=%f", cfg.WeatherURL, cfg.WeatherLocationLat, cfg.WeatherLocationLon)

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

	if status != http.StatusOK {
		return []Forecast{}, fmt.Errorf("Unexpected HTTP status %d:", status)
	}

	var respJSON forecastResponse

	if err := json.Unmarshal(body, &respJSON); err != nil {
		return []Forecast{}, fmt.Errorf("Error unmarshaling JSON: %w", err)
	}

	forecasts := make([]Forecast, 0, len(respJSON.Properties.Timeseries))

	for _, forecast := range respJSON.Properties.Timeseries {
		forecasts = append(forecasts, Forecast{
			Time:    forecast.Time,
			Instant: forecast.Data.Instant.Details,
			OneHour: PeriodForecast{
				SymbolCode:          forecast.Data.OneHour.Summary.SymbolCode,
				PrecipitationAmount: forecast.Data.OneHour.Details.PrecipitationAmount,
			},
			SixHours: PeriodForecast{
				SymbolCode:          forecast.Data.SixHours.Summary.SymbolCode,
				PrecipitationAmount: forecast.Data.SixHours.Details.PrecipitationAmount,
			},
			TwelveHours: PeriodForecast{
				SymbolCode: forecast.Data.TwelveHours.Summary.SymbolCode,
			},
		})
	}

	return forecasts, nil
}
