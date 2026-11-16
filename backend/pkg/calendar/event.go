package calendar

import (
	"strings"
	"time"
)

type Event struct {
	Start       time.Time `json:"start"`
	End         time.Time `json:"end"`
	ID          string    `json:"id"`
	Title       string    `json:"title"`
	Description string    `json:"description"`
	Location    string    `json:"location"`
	AllDay      bool      `json:"all_day"`
}

// Events is a series ordered ascending by start time.
type Events []Event

// Limit returns at most n events from the front of the series.
func (e Events) Limit(n int) Events {
	if len(e) < n {
		return e
	}

	return e[:n]
}

// Dedupe drops events repeating an earlier one's title, start and end.
func (e Events) Dedupe() Events {
	type key struct {
		title      string
		start, end int64
	}

	seen := make(map[key]bool, len(e))
	out := make(Events, 0, len(e))

	for _, ev := range e {
		k := key{ev.Title, ev.Start.UnixNano(), ev.End.UnixNano()}
		if seen[k] {
			continue
		}

		seen[k] = true
		out = append(out, ev)
	}

	return out
}

// Exclude drops events whose title matches one of titles, ignoring case and
// surrounding space.
func (e Events) Exclude(titles []string) Events {
	if len(titles) == 0 {
		return e
	}

	excluded := make(map[string]bool, len(titles))
	for _, t := range titles {
		excluded[normalizeTitle(t)] = true
	}

	out := make(Events, 0, len(e))

	for _, ev := range e {
		if !excluded[normalizeTitle(ev.Title)] {
			out = append(out, ev)
		}
	}

	return out
}

func normalizeTitle(s string) string {
	return strings.ToLower(strings.TrimSpace(s))
}
