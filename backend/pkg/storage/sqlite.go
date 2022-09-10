package storage

import (
	"context"
	"database/sql"
	"log"

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
		log.Println(e)
		if _, err := stmt.ExecContext(ctx, e.ID, e.Start.Unix(), e.End.Unix(), e.Title, e.Description, e.Location); err != nil {
			tx.Rollback()
			return err
		}
	}

	return tx.Commit()
}

func (c *SQLite) Close() {
	c.db.Close()
}
