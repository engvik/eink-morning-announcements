package storage

import (
	"database/sql"
	"path/filepath"
	"testing"
	"time"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
	"github.com/engvik/eink-morning-announcements/backend/pkg/message"
)

func newTestSQLite(t *testing.T, path string) *SQLite {
	t.Helper()

	c, err := NewSQLiteClient(&config.Config{SQLitePath: path, Location: "UTC"})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(c.Close)

	return c
}

func TestSQLiteMigratesExistingMessages(t *testing.T) {
	path := filepath.Join(t.TempDir(), "test.db")

	// A database as migration 001 left it.
	db, err := sql.Open("sqlite", path)
	if err != nil {
		t.Fatal(err)
	}

	for _, q := range []string{
		"CREATE TABLE messages (time INTEGER, message TEXT)",
		"INSERT INTO messages VALUES (1, 'old')",
		"PRAGMA user_version = 1",
	} {
		if _, err := db.Exec(q); err != nil {
			t.Fatal(err)
		}
	}
	db.Close()

	c := newTestSQLite(t, path)

	got, err := c.GetMessage(t.Context())
	if err != nil {
		t.Fatal(err)
	}

	if got.Message != "old" || got.Source != message.SourceManual {
		t.Errorf("GetMessage = %+v, want old manual message", got)
	}
}

func TestSQLiteMessages(t *testing.T) {
	c := newTestSQLite(t, filepath.Join(t.TempDir(), "test.db"))
	ctx := t.Context()

	if _, err := c.GetMessage(ctx); err != message.ErrNoMessages {
		t.Fatalf("GetMessage on empty = %v, want ErrNoMessages", err)
	}

	// Manual messages have no external ID and never collide.
	for range 2 {
		if err := c.SetMessage(ctx, message.Message{Message: "", Source: message.SourceManual}); err != nil {
			t.Fatal(err)
		}
	}

	if has, err := c.HasMessages(ctx, message.SourceMyKid); err != nil || has {
		t.Fatalf("HasMessages(mykid) = %v, %v; want false", has, err)
	}

	notice := message.Message{
		Time:       time.Now().Add(time.Hour),
		Message:    "Ny beskjed fra barnehagen (14.11)",
		Source:     message.SourceMyKid,
		ExternalID: "900001",
	}

	for i, want := range []bool{true, false} {
		added, err := c.AddExternalMessage(ctx, notice)
		if err != nil {
			t.Fatal(err)
		}

		if added != want {
			t.Errorf("AddExternalMessage #%d added = %v, want %v", i, added, want)
		}
	}

	if has, err := c.HasMessages(ctx, message.SourceMyKid); err != nil || !has {
		t.Fatalf("HasMessages(mykid) = %v, %v; want true", has, err)
	}

	got, err := c.GetMessage(ctx)
	if err != nil {
		t.Fatal(err)
	}

	if got.Message != notice.Message || got.Source != message.SourceMyKid {
		t.Errorf("GetMessage = %+v, want the notice", got)
	}

	if _, err := c.AddExternalMessage(ctx, message.Message{Source: message.SourceMyKid}); err == nil {
		t.Error("AddExternalMessage without ID: err = nil")
	}
}
