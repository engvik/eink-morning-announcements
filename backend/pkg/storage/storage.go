package storage

import (
	"context"
	"fmt"

	"github.com/engvik/eink-morning-announcements/backend/pkg/calendar"
	"github.com/engvik/eink-morning-announcements/backend/pkg/message"
	"github.com/engvik/eink-morning-announcements/backend/pkg/weather"
)

type store interface {
	SetCalendarEvents(context.Context, calendar.Events) error
	GetCalendarEvents(context.Context) (calendar.Events, error)
	SetWeatherForecasts(context.Context, weather.Forecasts) error
	GetWeatherForecasts(context.Context) (weather.Forecasts, error)
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

func (s *Storage) SetCalendarEvents(ctx context.Context, events calendar.Events) error {
	if len(events) == 0 {
		return fmt.Errorf("no data")
	}

	return s.client.SetCalendarEvents(ctx, events)
}

func (s *Storage) GetCalendarEvents(ctx context.Context) (calendar.Events, error) {
	return s.client.GetCalendarEvents(ctx)
}

func (s *Storage) SetWeatherForecasts(ctx context.Context, forecasts weather.Forecasts) error {
	if len(forecasts) == 0 {
		return fmt.Errorf("no data")
	}

	return s.client.SetWeatherForecasts(ctx, forecasts)
}

func (s *Storage) GetWeatherForecasts(ctx context.Context) (weather.Forecasts, error) {
	return s.client.GetWeatherForecasts(ctx)
}

func (s *Storage) SetMessage(ctx context.Context, m message.Message) error {
	if err := m.Valid(); err != nil {
		return err
	}

	return s.client.SetMessage(ctx, m)
}

func (s *Storage) GetMessage(ctx context.Context) (message.Message, error) {
	return s.client.GetMessage(ctx)
}
