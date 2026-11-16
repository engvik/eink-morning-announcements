package calendar

import (
	"context"
	"fmt"
	"log"
	"time"

	"golang.org/x/sync/errgroup"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
)

type fetcher interface {
	Fetch(context.Context, string) ([]byte, error)
}

type parser interface {
	Parse(string) (Events, error)
}

type store interface {
	SetCalendarEvents(context.Context, Events) error
	GetCalendarEvents(context.Context) (Events, error)
}

type Task struct {
	Fetcher        fetcher
	Parser         parser
	Storage        store
	feeds          config.Feeds
	exclude        []string
	excludeFeed    string
	updateInterval time.Duration
	timeout        time.Duration
}

func NewTask(cfg *config.Config, s store, f fetcher, p parser) *Task {
	return &Task{
		Fetcher:        f,
		Parser:         p,
		Storage:        s,
		feeds:          cfg.CalendarFeeds,
		exclude:        cfg.CalendarExclude,
		excludeFeed:    cfg.CalendarExcludeFeed,
		updateInterval: cfg.CalendarUpdateInterval,
		timeout:        cfg.CalendarTimeout,
	}
}

func (t *Task) Name() string {
	return "Calendar"
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

	feeds := make([]Events, len(t.feeds))

	// Any failed feed keeps the previous set, rather than dropping that feed's events.
	g, gctx := errgroup.WithContext(ctx)

	for i, feed := range t.feeds {
		g.Go(func() error {
			body, err := t.Fetcher.Fetch(gctx, feed.URL)
			if err != nil {
				return fmt.Errorf("fetching calendar %q: %w", feed.Label, err)
			}

			events, err := t.Parser.Parse(string(body))
			if err != nil {
				return fmt.Errorf("parsing calendar %q: %w", feed.Label, err)
			}

			// Exclude per feed, before the merge, so an excluded title in one
			// feed does not drop the same title from another.
			if t.excludeFeed == "" || feed.Label == t.excludeFeed {
				events = events.Exclude(t.exclude)
			}

			feeds[i] = events

			return nil
		})
	}

	if err := g.Wait(); err != nil {
		log.Printf("Error updating calendar: %s\n", err)
		return
	}

	var events Events
	for _, f := range feeds {
		events = append(events, f...)
	}

	events = events.Dedupe()

	if err := t.Storage.SetCalendarEvents(ctx, events); err != nil {
		log.Printf("Error storing calendar events: %s\n", err)
	}
}
