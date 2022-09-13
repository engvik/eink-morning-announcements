package weather

import (
	"context"
	"io"
	"log"
	"net/http"
	"time"
)

type fetcher interface {
	Fetch(context.Context) ([]Forecast, error)
}

type store interface {
	SetWeatherForecasts(context.Context, []Forecast) error
	GetWeatherForecasts(context.Context) ([]Forecast, error)
}

type Weather struct {
	Fetcher fetcher
	Storage store
}

func NewTask(f fetcher, s store) *Weather {
	return &Weather{
		Fetcher: f,
		Storage: s,
	}
}

func (w *Weather) Name() string {
	return "Weather"
}

func (w *Weather) Run() {
	ticker := time.NewTicker(time.Second * 5)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
			ctx := context.Background()

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
