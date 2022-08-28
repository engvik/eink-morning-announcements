package storage

import (
	"database/sql"

	_ "github.com/mattn/go-sqlite3"

	"github.com/engvik/eink/backend/internal/config"
)

type SQLite struct {
	DB *sql.DB
}

func NewSQLiteClient(cfg *config.Config) (*SQLite, error) {
	db, err := sql.Open("sqlite3", cfg.SQLitePath)
	if err != nil {
		return nil, err
	}

	return &SQLite{DB: db}, nil
}

func (c *SQLite) SetCalendarEvents() {
}

func (c *SQLite) GetCalendarEvents() {
}

func (c *SQLite) Close() {
	c.DB.Close()
}
