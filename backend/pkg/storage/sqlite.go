package storage

import (
	"context"
	"database/sql"
	"embed"
	"errors"
	"fmt"
	"io/fs"
	"slices"
	"time"

	_ "modernc.org/sqlite"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
	"github.com/engvik/eink-morning-announcements/backend/pkg/message"
)

//go:embed migrations/*.sql
var migrationsFS embed.FS

type SQLite struct {
	db       *sql.DB
	location *time.Location
}

func NewSQLiteClient(cfg *config.Config) (*SQLite, error) {
	db, err := sql.Open("sqlite", cfg.SQLitePath)
	if err != nil {
		return nil, err
	}

	location, err := time.LoadLocation(cfg.Location)
	if err != nil {
		return nil, err
	}

	c := &SQLite{
		db:       db,
		location: location,
	}

	if err := c.runMigrations(); err != nil {
		db.Close()
		return nil, fmt.Errorf("migrate: %w", err)
	}

	return c, nil
}

// runMigrations applies every migration the database has not seen yet, in
// filename order. PRAGMA user_version holds the number applied so far.
func (c *SQLite) runMigrations() error {
	var version int
	if err := c.db.QueryRow("PRAGMA user_version").Scan(&version); err != nil {
		return fmt.Errorf("read user_version: %w", err)
	}

	entries, err := fs.Glob(migrationsFS, "migrations/*.sql")
	if err != nil {
		return fmt.Errorf("list migrations: %w", err)
	}

	slices.Sort(entries)

	for i, entry := range entries {
		next := i + 1
		if next <= version {
			continue
		}

		if err := c.migrate(entry, next); err != nil {
			return err
		}
	}

	return nil
}

// migrate applies one migration and bumps user_version in the same
// transaction, so a failure leaves the database untouched.
func (c *SQLite) migrate(entry string, next int) error {
	data, err := fs.ReadFile(migrationsFS, entry)
	if err != nil {
		return fmt.Errorf("read migration %s: %w", entry, err)
	}

	tx, err := c.db.Begin()
	if err != nil {
		return fmt.Errorf("begin migration %s: %w", entry, err)
	}

	if _, err := tx.Exec(string(data)); err != nil {
		if rbErr := tx.Rollback(); rbErr != nil {
			return fmt.Errorf("rollback migration %s: %w", entry, rbErr)
		}

		return fmt.Errorf("exec migration %s: %w", entry, err)
	}

	if _, err := tx.Exec(fmt.Sprintf("PRAGMA user_version = %d", next)); err != nil {
		if rbErr := tx.Rollback(); rbErr != nil {
			return fmt.Errorf("rollback migration %s: %w", entry, rbErr)
		}

		return fmt.Errorf("set user_version after %s: %w", entry, err)
	}

	return tx.Commit()
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

	err := c.db.QueryRowContext(ctx, "SELECT time, message FROM messages ORDER BY time DESC LIMIT 1").Scan(&timestamp, &msg)
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
