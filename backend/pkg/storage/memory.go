package storage

import (
	"context"
	"slices"
	"sync"
	"time"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
	"github.com/engvik/eink-morning-announcements/backend/pkg/calendar"
	"github.com/engvik/eink-morning-announcements/backend/pkg/weather"
)

// Memory holds calendar events and weather forecasts. Both are re-fetchable
// from their upstreams, so they are cached rather than persisted. Each fetch
// replaces the previous set, which is also what keeps past entries from
// accumulating.
type Memory struct {
	mu        sync.RWMutex
	events    []calendar.Event
	forecasts []weather.Forecast

	location                 *time.Location
	numCalendarFetchEvents   int
	numWeatherFetchForecasts int
}

func NewMemoryClient(cfg *config.Config) (*Memory, error) {
	location, err := time.LoadLocation(cfg.Location)
	if err != nil {
		return nil, err
	}

	return &Memory{
		location:                 location,
		numCalendarFetchEvents:   cfg.CalendarFetchEvents,
		numWeatherFetchForecasts: cfg.WeatherFetchEorecasts,
	}, nil
}

func (c *Memory) SetCalendarEvents(_ context.Context, events []calendar.Event) error {
	slices.SortFunc(events, func(a, b calendar.Event) int {
		return a.Start.Compare(b.Start)
	})

	c.mu.Lock()
	defer c.mu.Unlock()

	c.events = events

	return nil
}

func (c *Memory) GetCalendarEvents(_ context.Context) ([]calendar.Event, error) {
	c.mu.RLock()
	defer c.mu.RUnlock()

	now := time.Now()
	events := make([]calendar.Event, 0, c.numCalendarFetchEvents)

	for _, e := range c.events {
		if e.Start.Before(now) {
			continue
		}

		if len(events) == c.numCalendarFetchEvents {
			break
		}

		e.Start = e.Start.In(c.location)
		e.End = e.End.In(c.location)

		events = append(events, e)
	}

	return events, nil
}

func (c *Memory) SetWeatherForecasts(_ context.Context, forecasts []weather.Forecast) error {
	slices.SortFunc(forecasts, func(a, b weather.Forecast) int {
		return a.Time.Compare(b.Time)
	})

	c.mu.Lock()
	defer c.mu.Unlock()

	c.forecasts = forecasts

	return nil
}

func (c *Memory) GetWeatherForecasts(_ context.Context) ([]weather.Forecast, error) {
	c.mu.RLock()
	defer c.mu.RUnlock()

	now := time.Now()
	forecasts := make([]weather.Forecast, 0, c.numWeatherFetchForecasts)

	for _, f := range c.forecasts {
		if f.Time.Before(now) {
			continue
		}

		if len(forecasts) == c.numWeatherFetchForecasts {
			break
		}

		f.Time = f.Time.In(c.location)

		forecasts = append(forecasts, f)
	}

	return forecasts, nil
}
