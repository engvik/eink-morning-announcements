package storage

import "github.com/engvik/eink-morning-announcements/backend/internal/config"

// Client is the storage backend passed to New. Calendar events and weather
// forecasts are cached in memory; the message is user-authored, cannot be
// fetched from anywhere, and is persisted.
type Client struct {
	*Memory
	*SQLite
}

func NewClient(cfg *config.Config) (*Client, error) {
	memory, err := NewMemoryClient(cfg)
	if err != nil {
		return nil, err
	}

	sqlite, err := NewSQLiteClient(cfg)
	if err != nil {
		return nil, err
	}

	return &Client{
		Memory: memory,
		SQLite: sqlite,
	}, nil
}
