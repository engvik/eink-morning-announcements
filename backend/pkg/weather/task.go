package weather

import (
	"context"
	"fmt"
	"log"
	"time"

	"golang.org/x/sync/errgroup"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
)

type fetcher interface {
	Fetch(context.Context) (Forecasts, error)
	FetchSun(context.Context) (Sun, error)
}

type store interface {
	SetWeatherForecasts(context.Context, Forecasts) error
	GetWeatherForecasts(context.Context) (Forecasts, error)
	SetSun(context.Context, Sun) error
	GetSun(context.Context) (Sun, error)
}

type Task struct {
	Fetcher        fetcher
	Storage        store
	updateInterval time.Duration
	timeout        time.Duration
}

func NewTask(cfg *config.Config, f fetcher, s store) *Task {
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

func (t *Task) Run(ctx context.Context) {
	ticker := time.NewTicker(t.updateInterval)
	defer ticker.Stop()

	// Update immediately, otherwise nothing is served until the first tick.
	t.update(ctx)

	for {
		select {
		case <-ctx.Done():
			return
		case <-ticker.C:
			t.update(ctx)
		}
	}
}

func (t *Task) update(ctx context.Context) {
	ctx, cancel := context.WithTimeout(ctx, t.timeout)
	defer cancel()

	// Forecasts and sunrise are available on separate MET APIs.
	var g errgroup.Group

	g.Go(func() error {
		forecasts, err := t.Fetcher.Fetch(ctx)
		if err != nil {
			return fmt.Errorf("fetching forecasts: %w", err)
		}

		if err := t.Storage.SetWeatherForecasts(ctx, forecasts); err != nil {
			return fmt.Errorf("storing forecasts: %w", err)
		}

		return nil
	})

	g.Go(func() error {
		sun, err := t.Fetcher.FetchSun(ctx)
		if err != nil {
			return fmt.Errorf("fetching sun: %w", err)
		}

		if err := t.Storage.SetSun(ctx, sun); err != nil {
			return fmt.Errorf("storing sun: %w", err)
		}

		return nil
	})

	if err := g.Wait(); err != nil {
		log.Printf("Error updating weather: %s\n", err)
	}
}
