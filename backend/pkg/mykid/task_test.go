package mykid

import (
	"context"
	"slices"
	"testing"
	"time"

	"github.com/engvik/eink-morning-announcements/backend/pkg/message"
)

type fakeNoticer map[string][]Notice

func (f fakeNoticer) Notices(_ context.Context, kid string) ([]Notice, error) {
	return f[kid], nil
}

// memStore mirrors the SQLite semantics the task relies on.
type memStore struct {
	msgs []message.Message
}

func (s *memStore) GetMessage(context.Context) (message.Message, error) {
	if len(s.msgs) == 0 {
		return message.Message{}, message.ErrNoMessages
	}

	return slices.MaxFunc(s.msgs, func(a, b message.Message) int { return a.Time.Compare(b.Time) }), nil
}

func (s *memStore) AddExternalMessage(_ context.Context, m message.Message) (bool, error) {
	for _, old := range s.msgs {
		if old.Source == m.Source && old.ExternalID == m.ExternalID {
			return false, nil
		}
	}

	s.msgs = append(s.msgs, m)

	return true, nil
}

func (s *memStore) HasMessages(_ context.Context, source string) (bool, error) {
	return slices.ContainsFunc(s.msgs, func(m message.Message) bool { return m.Source == source }), nil
}

func TestTaskUpdate(t *testing.T) {
	day := func(d int) time.Time { return time.Date(2026, 11, d, 0, 0, 0, 0, time.UTC) }
	now := day(14).Add(16 * time.Hour)

	shared := Notice{ID: "1", Date: day(9), Heading: "Barnehagen"}
	noticer := fakeNoticer{"a": {shared}, "b": {shared}}

	s := &memStore{msgs: []message.Message{
		{Time: day(10), Message: "Ta med sekk", Source: message.SourceManual},
	}}

	task := &Task{
		Client:  noticer,
		Storage: s,
		kids:    []string{"a", "b"},
		timeout: time.Second,
		now:     func() time.Time { return now },
	}

	// First run back-dates the backlog, so the newer manual reminder stays on top.
	task.update(t.Context())

	if len(s.msgs) != 2 {
		t.Fatalf("stored %d messages, want 2 (shared post-it once)", len(s.msgs))
	}

	if got, _ := s.GetMessage(t.Context()); got.Source != message.SourceManual {
		t.Errorf("latest after seeding = %+v, want the manual reminder", got)
	}

	// A new post-it on a later run takes the slot at the current time.
	noticer["b"] = append(noticer["b"], Notice{ID: "2", Date: day(14), Heading: "Avdelingen"})
	task.update(t.Context())

	got, _ := s.GetMessage(t.Context())
	want := message.Message{Time: now, Message: "Ny beskjed fra barnehagen (14.11)", Source: message.SourceMyKid, ExternalID: "2"}

	if got != want {
		t.Errorf("latest = %+v, want %+v", got, want)
	}

	// Unchanged post-its are not stored again.
	task.update(t.Context())

	if len(s.msgs) != 3 {
		t.Errorf("stored %d messages, want 3", len(s.msgs))
	}
}
