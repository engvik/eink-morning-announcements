package storage

import (
	"context"
	"errors"

	"github.com/engvik/eink/backend/pkg/calendar"
	"github.com/engvik/eink/backend/pkg/weather"
)

var (
	ErrNoData = errors.New("No data passed")
)

type store interface {
	SetCalendarEvents(context.Context, []calendar.Event) error
	GetCalendarEvents(context.Context) ([]calendar.Event, error)
	SetWeatherForecasts(context.Context, []weather.Forecast) error
	GetWeatherForecasts(context.Context) ([]weather.Forecast, error)
}

type Storage struct {
	client store
}

func New(c store) *Storage {
	return &Storage{
		client: c,
	}
}

func (s *Storage) SetCalendarEvents(ctx context.Context, events []calendar.Event) error {
	if len(events) == 0 {
		return ErrNoData
	}

	return s.client.SetCalendarEvents(ctx, events)
}

func (s *Storage) GetCalendarEvents(ctx context.Context) ([]calendar.Event, error) {
	return s.client.GetCalendarEvents(ctx)
}

func (s *Storage) SetWeatherForecasts(ctx context.Context, forecasts []weather.Forecast) error {
	if len(forecasts) == 0 {
		return ErrNoData
	}

	return s.client.SetWeatherForecasts(ctx, forecasts)
}

func (s *Storage) GetWeatherForecasts(ctx context.Context) ([]weather.Forecast, error) {
	return s.client.GetWeatherForecasts(ctx)
}
