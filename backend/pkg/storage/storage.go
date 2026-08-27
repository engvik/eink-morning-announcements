package storage

import (
	"context"
	"database/sql"
	"errors"
	"fmt"

	"github.com/engvik/eink-morning-announcements/backend/pkg/calendar"
	"github.com/engvik/eink-morning-announcements/backend/pkg/message"
	"github.com/engvik/eink-morning-announcements/backend/pkg/weather"
)

type store interface {
	SetCalendarEvents(context.Context, []calendar.Event) error
	GetCalendarEvents(context.Context) ([]calendar.Event, error)
	SetWeatherForecasts(context.Context, []weather.Forecast) error
	GetWeatherForecasts(context.Context) ([]weather.Forecast, error)
	SetMessage(context.Context, message.Message) error
	GetMessage(context.Context) (message.Message, error)
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
		return fmt.Errorf("no data")
	}

	return s.client.SetCalendarEvents(ctx, events)
}

func (s *Storage) GetCalendarEvents(ctx context.Context) ([]calendar.Event, error) {
	return s.client.GetCalendarEvents(ctx)
}

func (s *Storage) SetWeatherForecasts(ctx context.Context, forecasts []weather.Forecast) error {
	if len(forecasts) == 0 {
		return fmt.Errorf("no data")
	}

	return s.client.SetWeatherForecasts(ctx, forecasts)
}

func (s *Storage) GetWeatherForecasts(ctx context.Context) ([]weather.Forecast, error) {
	return s.client.GetWeatherForecasts(ctx)
}

func (s *Storage) SetMessage(ctx context.Context, m message.Message) error {
	if err := m.Valid(); err != nil {
		return err
	}

	return s.client.SetMessage(ctx, m)
}

func (s *Storage) GetMessage(ctx context.Context) (message.Message, error) {
	m, err := s.client.GetMessage(ctx)
	if errors.Is(err, sql.ErrNoRows) {
		return m, message.ErrNoMessages
	} else if err != nil {
		return m, err
	}

	return m, nil
}
