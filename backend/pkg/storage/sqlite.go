package storage

import (
	"context"
	"database/sql"
	"errors"
	"time"

	_ "github.com/mattn/go-sqlite3"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
	"github.com/engvik/eink-morning-announcements/backend/pkg/message"
)

type SQLite struct {
	db       *sql.DB
	location *time.Location
}

func NewSQLiteClient(cfg *config.Config) (*SQLite, error) {
	db, err := sql.Open("sqlite3", cfg.SQLitePath)
	if err != nil {
		return nil, err
	}

	location, err := time.LoadLocation(cfg.Location)
	if err != nil {
		return nil, err
	}

	return &SQLite{
		db:       db,
		location: location,
	}, nil
}

func (c *SQLite) SetMessage(ctx context.Context, m message.Message) error {
	stmt, err := c.db.PrepareContext(
		ctx,
		`
		INSERT OR REPLACE INTO messages
		VALUES (?, ?)
		`,
	)
	if err != nil {
		return err
	}

	if _, err := stmt.ExecContext(ctx, time.Now().UnixMicro(), m.Message); err != nil {
		return err
	}

	return nil
}

func (c *SQLite) GetMessage(ctx context.Context) (message.Message, error) {
	var timestamp int64
	var msg string

	err := c.db.QueryRowContext(ctx, "SELECT * FROM messages ORDER BY time DESC LIMIT 1").Scan(&timestamp, &msg)
	if errors.Is(err, sql.ErrNoRows) {
		return message.Message{}, message.ErrNoMessages
	}

	if err != nil {
		return message.Message{}, err
	}

	return message.Message{
		Time:    time.UnixMicro(timestamp).In(c.location),
		Message: msg,
	}, nil
}

func (c *SQLite) Close() {
	c.db.Close()
}
