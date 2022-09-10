package storage

import (
	"context"

	"github.com/engvik/eink/backend/pkg/calendar"
)

type store interface {
	SetCalendarEvents(context.Context, []calendar.Event) error
	GetCalendarEvents(context.Context) ([]calendar.Event, error)
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
	return s.client.SetCalendarEvents(ctx, events)
}

func (s *Storage) GetCalendarEvents(ctx context.Context) ([]calendar.Event, error) {
	return s.client.GetCalendarEvents(ctx)
}
