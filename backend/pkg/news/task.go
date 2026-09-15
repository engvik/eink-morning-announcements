package news

import (
	"context"
	"log"
	"sync"
	"time"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
)

type fetcher interface {
	Fetch(context.Context, string) ([]byte, error)
}

type parser interface {
	Parse([]byte, string) (Items, error)
}

type store interface {
	SetNews(context.Context, Items) error
	GetNews(context.Context) (Items, error)
}

type Task struct {
	Fetcher        fetcher
	Parser         parser
	Storage        store
	feeds          []config.Feed
	updateInterval time.Duration
	timeout        time.Duration
}

func NewTask(cfg *config.Config, s store, f fetcher, p parser) *Task {
	return &Task{
		Fetcher:        f,
		Parser:         p,
		Storage:        s,
		feeds:          cfg.NewsFeeds,
		updateInterval: cfg.NewsUpdateInterval,
		timeout:        cfg.NewsTimeout,
	}
}

func (t *Task) Name() string {
	return "News"
}

func (t *Task) Run(ctx context.Context) {
	if len(t.feeds) == 0 {
		log.Println("No news feeds configured")
		return
	}

	ticker := time.NewTicker(t.updateInterval)
	defer ticker.Stop()

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

	feeds := make([]Items, len(t.feeds))

	var wg sync.WaitGroup

	for i, feed := range t.feeds {
		wg.Go(func() {
			body, err := t.Fetcher.Fetch(ctx, feed.URL)
			if err != nil {
				log.Printf("Error fetching news from %s: %s\n", feed.Label, err)
				return
			}

			items, err := t.Parser.Parse(body, feed.Label)
			if err != nil {
				log.Printf("Error parsing news from %s: %s\n", feed.Label, err)
				return
			}

			feeds[i] = items
		})
	}

	wg.Wait()

	interleaved := interleave(feeds)

	if err := t.Storage.SetNews(ctx, interleaved); err != nil {
		log.Printf("Error storing news: %s\n", err)
	}
}
