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

type memStore struct {
	msgs []storedMsg
}

type storedMsg struct {
	m      message.Message
	active bool
}

func (s *memStore) GetMessage(context.Context) (message.Message, error) {
	var latest message.Message
	found := false

	for _, row := range s.msgs {
		if row.active && (!found || row.m.Time.After(latest.Time)) {
			latest = row.m
			found = true
		}
	}

	if !found {
		return message.Message{}, message.ErrNoMessages
	}

	return latest, nil
}

func (s *memStore) AddExternalMessage(_ context.Context, m message.Message, show bool) (bool, error) {
	for _, old := range s.msgs {
		if old.m.Source == m.Source && old.m.ExternalID == m.ExternalID {
			return false, nil
		}
	}

	s.msgs = append(s.msgs, storedMsg{m: m, active: show})

	return true, nil
}

func (s *memStore) HasMessages(_ context.Context, source string) (bool, error) {
	return slices.ContainsFunc(s.msgs, func(r storedMsg) bool { return r.m.Source == source }), nil
}

func (s *memStore) DeactivateExternalMessages(_ context.Context, source string) error {
	for i := range s.msgs {
		if s.msgs[i].m.Source == source {
			s.msgs[i].active = false
		}
	}

	return nil
}

func (s *memStore) setManual(m message.Message) {
	s.msgs = append(s.msgs, storedMsg{m: m, active: true})
}

func TestTaskUpdate(t *testing.T) {
	day := func(d int) time.Time { return time.Date(2026, 11, d, 0, 0, 0, 0, time.UTC) }
	now := day(14).Add(16 * time.Hour)

	shared := Notice{ID: "1", Date: day(9), Heading: "Barnehagen"}
	noticer := fakeNoticer{"a": {shared}, "b": {shared}}

	s := &memStore{}
	s.setManual(message.Message{Time: day(10), Message: "Ta med sekk", Source: message.SourceManual})

	task := &Task{
		Client:  noticer,
		Storage: s,
		kids:    []string{"a", "b"},
		timeout: time.Second,
		now:     func() time.Time { return now },
	}

	mustShow := func(label, wantSource, wantMsg string) {
		t.Helper()
		got, _ := s.GetMessage(t.Context())
		if got.Source != wantSource || got.Message != wantMsg {
			t.Errorf("%s: reminder = %+v, want source %q message %q", label, got, wantSource, wantMsg)
		}
	}

	// First run seeds the existing backlog silently: nothing shown, manual stands.
	task.update(t.Context())
	mustShow("after first-run seed", message.SourceManual, "Ta med sekk")

	// A new post-it is shown on the run it first appears.
	noticer["b"] = append(noticer["b"], Notice{ID: "2", Date: day(14), Heading: "Avdelingen"})
	task.update(t.Context())
	mustShow("new notice", message.SourceMyKid, "Ny beskjed fra barnehagen (14.11)")

	// Next scheduled run with no newer post-it clears it back to the manual message.
	task.update(t.Context())
	mustShow("notice expired", message.SourceManual, "Ta med sekk")

	// And it is not resurrected on later runs while it is still the newest post-it.
	task.update(t.Context())
	mustShow("stays cleared", message.SourceManual, "Ta med sekk")

	// A genuinely new post-it shows again.
	noticer["a"] = append(noticer["a"], Notice{ID: "3", Date: day(15), Heading: "Barnehagen"})
	task.update(t.Context())
	mustShow("another new notice", message.SourceMyKid, "Ny beskjed fra barnehagen (15.11)")
}
