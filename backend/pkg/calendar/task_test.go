package calendar

import (
	"context"
	"errors"
	"slices"
	"testing"
	"time"
)

type fakeFetcher map[string]error

func (f fakeFetcher) Fetch(_ context.Context, url string) ([]byte, error) {
	return []byte(url), f[url]
}

// fakeParser treats the body as the feed URL and returns that feed's events.
type fakeParser map[string]Events

func (p fakeParser) Parse(body string) (Events, error) {
	return p[body], nil
}

type fakeStore struct {
	events Events
	calls  int
}

func (s *fakeStore) SetCalendarEvents(_ context.Context, e Events) error {
	s.events = e
	s.calls++
	return nil
}

func (s *fakeStore) GetCalendarEvents(context.Context) (Events, error) {
	return s.events, nil
}

func TestTaskUpdate(t *testing.T) {
	start := time.Date(2026, 11, 16, 10, 0, 0, 0, time.UTC)
	end := start.Add(time.Hour)

	feeds := fakeParser{
		"ical": {{ID: "home", Title: "Tannlege", Start: start, End: end}},
		"mykid": {
			{ID: "531853_1", Title: "Tur", Start: start, End: end},
			{ID: "547103_1", Title: "Tur", Start: start, End: end},
			{ID: "531853_2", Title: "Første måltid", Start: start, End: end},
		},
	}

	tests := []struct {
		name      string
		fetchErrs fakeFetcher
		wantIDs   []string
		wantCalls int
	}{
		{
			name:      "merges, dedupes and excludes",
			fetchErrs: fakeFetcher{},
			wantIDs:   []string{"home", "531853_1"},
			wantCalls: 1,
		},
		{
			name:      "one failing feed keeps the previous set",
			fetchErrs: fakeFetcher{"mykid": errors.New("boom")},
			wantIDs:   []string{"previous"},
			wantCalls: 0,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			s := &fakeStore{events: Events{{ID: "previous"}}}
			task := &Task{
				Fetcher: tt.fetchErrs,
				Parser:  feeds,
				Storage: s,
				urls:    []string{"ical", "mykid"},
				exclude: []string{"Første måltid"},
				timeout: time.Second,
			}

			task.update(t.Context())

			if s.calls != tt.wantCalls {
				t.Errorf("SetCalendarEvents calls = %d, want %d", s.calls, tt.wantCalls)
			}

			if got := ids(s.events); !slices.Equal(got, tt.wantIDs) {
				t.Errorf("stored = %v, want %v", got, tt.wantIDs)
			}
		})
	}
}
