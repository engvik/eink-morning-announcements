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
	_, err := c.db.ExecContext(
		ctx,
		"INSERT INTO messages (time, message, source) VALUES (?, ?, ?)",
		time.Now().UnixMicro(), m.Message, m.Source,
	)

	return err
}

// AddExternalMessage stores m at m.Time unless its source and external ID are
// already stored, and reports whether it was added.
func (c *SQLite) AddExternalMessage(ctx context.Context, m message.Message) (bool, error) {
	if m.ExternalID == "" {
		return false, errors.New("external message without external ID")
	}

	res, err := c.db.ExecContext(
		ctx,
		"INSERT OR IGNORE INTO messages (time, message, source, external_id) VALUES (?, ?, ?, ?)",
		m.Time.UnixMicro(), m.Message, m.Source, m.ExternalID,
	)
	if err != nil {
		return false, err
	}

	n, err := res.RowsAffected()
	if err != nil {
		return false, err
	}

	return n == 1, nil
}

func (c *SQLite) HasMessages(ctx context.Context, source string) (bool, error) {
	var exists bool

	err := c.db.QueryRowContext(ctx, "SELECT EXISTS (SELECT 1 FROM messages WHERE source = ?)", source).Scan(&exists)

	return exists, err
}

func (c *SQLite) GetMessage(ctx context.Context) (message.Message, error) {
	var timestamp int64
	var msg, source string

	err := c.db.QueryRowContext(ctx, "SELECT time, message, source FROM messages ORDER BY time DESC LIMIT 1").Scan(&timestamp, &msg, &source)
	if errors.Is(err, sql.ErrNoRows) {
		return message.Message{}, message.ErrNoMessages
	}

	if err != nil {
		return message.Message{}, err
	}

	return message.Message{
		Time:    time.UnixMicro(timestamp).In(c.location),
		Message: msg,
		Source:  source,
	}, nil
}

func (c *SQLite) Close() {
	c.db.Close()
}
