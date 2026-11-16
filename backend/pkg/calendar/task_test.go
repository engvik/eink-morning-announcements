package calendar

import (
	"context"
	"errors"
	"slices"
	"testing"
	"time"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
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

	// Keyed by feed URL. Both feeds carry Yoga, so dedupe and scoped exclude interact.
	feeds := fakeParser{
		"ical": {
			{ID: "home", Title: "Tannlege", Start: start, End: end},
			{ID: "home_yoga", Title: "Yoga", Start: start, End: end},
		},
		"mykid": {
			{ID: "kid1_tur", Title: "Tur", Start: start, End: end},
			{ID: "kid2_tur", Title: "Tur", Start: start, End: end},
			{ID: "kid1_yoga", Title: "Yoga", Start: start, End: end},
		},
	}

	calendarFeeds := config.Feeds{{Label: "personal", URL: "ical"}, {Label: "mykid", URL: "mykid"}}

	tests := []struct {
		name        string
		fetchErrs   fakeFetcher
		exclude     []string
		excludeFeed string
		wantIDs     []string
		wantCalls   int
	}{
		{
			name:      "merges and dedupes, no exclude",
			fetchErrs: fakeFetcher{},
			wantIDs:   []string{"home", "home_yoga", "kid1_tur"},
			wantCalls: 1,
		},
		{
			name:        "exclude scoped to one feed keeps the title in the other",
			fetchErrs:   fakeFetcher{},
			exclude:     []string{"Yoga"},
			excludeFeed: "mykid",
			wantIDs:     []string{"home", "home_yoga", "kid1_tur"},
			wantCalls:   1,
		},
		{
			name:        "feed label match ignores case and space",
			fetchErrs:   fakeFetcher{},
			exclude:     []string{"Yoga"},
			excludeFeed: " MyKid ",
			wantIDs:     []string{"home", "home_yoga", "kid1_tur"},
			wantCalls:   1,
		},
		{
			name:      "no feed scope excludes everywhere",
			fetchErrs: fakeFetcher{},
			exclude:   []string{"Yoga"},
			wantIDs:   []string{"home", "kid1_tur"},
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
				Fetcher:     tt.fetchErrs,
				Parser:      feeds,
				Storage:     s,
				feeds:       calendarFeeds,
				exclude:     tt.exclude,
				excludeFeed: tt.excludeFeed,
				timeout:     time.Second,
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
