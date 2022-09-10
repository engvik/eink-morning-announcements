package storage

import (
	"context"
	"database/sql"
	"fmt"
	"time"

	_ "github.com/mattn/go-sqlite3"

	"github.com/engvik/eink/backend/internal/config"
	"github.com/engvik/eink/backend/pkg/calendar"
)

type SQLite struct {
	db *sql.DB
}

func NewSQLiteClient(cfg *config.Config) (*SQLite, error) {
	db, err := sql.Open("sqlite3", cfg.SQLitePath)
	if err != nil {
		return nil, err
	}

	return &SQLite{db: db}, nil
}

func (c *SQLite) SetCalendarEvents(ctx context.Context, events []calendar.Event) error {
	tx, err := c.db.BeginTx(ctx, nil)
	if err != nil {
		return err
	}

	stmt, err := tx.PrepareContext(
		ctx,
		`
		INSERT OR REPLACE INTO events
		VALUES (?, ?, ?, ?, ?, ?)
		`,
	)
	if err != nil {
		tx.Rollback()
		return err
	}

	for _, e := range events {
		if _, err := stmt.ExecContext(ctx, e.ID, e.Start.UnixMicro(), e.End.UnixMicro(), e.Title, e.Description, e.Location); err != nil {
			tx.Rollback()
			return err
		}
	}

	return tx.Commit()
}

func (c *SQLite) GetCalendarEvents(ctx context.Context) ([]calendar.Event, error) {
	rows, err := c.db.QueryContext(
		ctx,
		`
		SELECT *
		FROM events
		ORDER BY start
		LIMIT 10
		`,
	)
	if err != nil {
		return []calendar.Event{}, err
	}

	defer rows.Close()

	events := make([]calendar.Event, 0, 10)

	for rows.Next() {
		var (
			id          string
			start       int64
			end         int64
			title       string
			description string
			location    string
		)

		if err := rows.Scan(&id, &start, &end, &title, &description, &location); err != nil {
			return events, fmt.Errorf("scan: %w", err)
		}

		events = append(
			events,
			calendar.Event{
				ID:          id,
				Start:       time.UnixMicro(start),
				End:         time.UnixMicro(end),
				Title:       title,
				Description: description,
				Location:    location,
			},
		)
	}

	if err := rows.Err(); err != nil {
		return events, err
	}

	return events, nil
}

func (c *SQLite) Close() {
	c.db.Close()
}
