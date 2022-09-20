package weather

import (
	"context"
	"log"
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

type Task struct {
	Fetcher        fetcher
	Storage        store
	updateInterval time.Duration
	timeout        time.Duration
}

func NewTask(f fetcher, s store, cfg *config.Config) *Task {
	return &Task{
		Fetcher:        f,
		Storage:        s,
		updateInterval: cfg.WeatherUpdateInterval,
		timeout:        cfg.WeatherTimeout,
	}
}

func (t *Task) Name() string {
	return "Weather"
}

func (t *Task) Run() {
	ticker := time.NewTicker(t.updateInterval)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
			ctx, cancel := context.WithTimeout(context.Background(), t.timeout)

			forecasts, err := t.Fetcher.Fetch(ctx)
			if err != nil {
				log.Printf("Error fetching weather data: %s\n", err)
				continue
			}

			if err := t.Storage.SetWeatherForecasts(ctx, forecasts); err != nil {
				log.Printf("Error storing weather forecast: %s\n", err)
			}

			cancel()
		}
	}
}
