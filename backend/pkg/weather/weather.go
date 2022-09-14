package weather

import (
	"context"
	"io"
	"log"
	"net/http"
	"time"

	"github.com/engvik/eink/backend/internal/config"
)

type fetcher interface {
	Fetch(context.Context) ([]Forecast, error)
}

type store interface {
	SetWeatherForecasts(context.Context, []Forecast) error
	GetWeatherForecasts(context.Context) ([]Forecast, error)
}

type Weather struct {
	Fetcher        fetcher
	Storage        store
	updateInterval time.Duration
	timeout        time.Duration
}

func NewTask(f fetcher, s store, cfg *config.Config) *Weather {
	return &Weather{
		Fetcher:        f,
		Storage:        s,
		updateInterval: cfg.WeatherUpdateInterval,
		timeout:        cfg.WeatherTimeout,
	}
}

func (w *Weather) Name() string {
	return "Weather"
}

func (w *Weather) Run() {
	ticker := time.NewTicker(time.Second * w.updateInterval)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
			ctx, cancel := context.WithTimeout(context.Background(), w.timeout)
			defer cancel()

			forecasts, err := w.Fetcher.Fetch(ctx)
			if err != nil {
				log.Printf("Error fetching weather data: %s\n", err)
				continue
			}

			if err := w.Storage.SetWeatherForecasts(ctx, forecasts); err != nil {
				log.Printf("Error storing weather forecast: %s\n", err)
			}
		}
	}
}

func readBody(resp *http.Response) ([]byte, error) {
	defer resp.Body.Close()

	return io.ReadAll(resp.Body)
}
