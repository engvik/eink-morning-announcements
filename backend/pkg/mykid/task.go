package mykid

import (
	"cmp"
	"context"
	"errors"
	"log"
	"slices"
	"time"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
	"github.com/engvik/eink-morning-announcements/backend/pkg/message"
)

type noticer interface {
	Notices(context.Context, string) ([]Notice, error)
}

type store interface {
	GetMessage(context.Context) (message.Message, error)
	AddExternalMessage(context.Context, message.Message, bool) (bool, error)
	HasMessages(context.Context, string) (bool, error)
	DeactivateExternalMessages(context.Context, string) error
}

// Task turns new post-its into a short reminder pointing at the app.
type Task struct {
	Client  noticer
	Storage store
	kids    []string
	times   config.Clocks
	timeout time.Duration
	now     func() time.Time
}

// NewTask updates at cfg.MyKidUpdateTimes in loc.
func NewTask(cfg *config.Config, loc *time.Location, c noticer, s store) *Task {
	return &Task{
		Client:  c,
		Storage: s,
		kids:    cfg.MyKidKids,
		times:   cfg.MyKidUpdateTimes,
		timeout: cfg.MyKidTimeout,
		now:     func() time.Time { return time.Now().In(loc) },
	}
}

func (t *Task) Name() string {
	return "MyKid"
}

func (t *Task) Run(ctx context.Context) {
	t.update(ctx)

	for {
		timer := time.NewTimer(time.Until(t.times.Next(t.now())))

		select {
		case <-ctx.Done():
			timer.Stop()
			return
		case <-timer.C:
			t.update(ctx)
		}
	}
}

func (t *Task) update(ctx context.Context) {
	ctx, cancel := context.WithTimeout(ctx, t.timeout)
	defer cancel()

	var notices []Notice

	for _, kid := range t.kids {
		n, err := t.Client.Notices(ctx, kid)
		if err != nil {
			log.Printf("Error fetching MyKid notices for %s: %s\n", kid, err)
			return
		}

		notices = append(notices, n...)
	}

	// Shared across kids, oldest first so the newest takes the slot.
	slices.SortFunc(notices, func(a, b Notice) int {
		return cmp.Or(a.Date.Compare(b.Date), cmp.Compare(a.ID, b.ID))
	})
	notices = slices.CompactFunc(notices, func(a, b Notice) bool { return a.ID == b.ID })

	// First run, remember the backlog without showing it.
	seeded, err := t.Storage.HasMessages(ctx, message.SourceMyKid)
	if err != nil {
		log.Printf("Error checking MyKid messages: %s\n", err)
		return
	}

	before, err := t.Storage.GetMessage(ctx)
	if err != nil && !errors.Is(err, message.ErrNoMessages) {
		log.Printf("Error getting message: %s\n", err)
		return
	}

	// Expire last run's notice.
	if err := t.Storage.DeactivateExternalMessages(ctx, message.SourceMyKid); err != nil {
		log.Printf("Error expiring MyKid notices: %s\n", err)
		return
	}

	for _, n := range notices {
		m := message.Message{
			Time:       t.now(),
			Message:    "Ny beskjed fra barnehagen (" + n.Date.Format("02.01") + ")",
			Source:     message.SourceMyKid,
			ExternalID: n.ID,
		}

		// Only brand-new notices are added, so each shows for one run.
		added, err := t.Storage.AddExternalMessage(ctx, m, seeded)
		if err != nil {
			log.Printf("Error storing MyKid notice %s: %s\n", n.ID, err)
			return
		}

		if added && seeded && before.Source == message.SourceManual && before.Message != "" {
			log.Printf("MyKid notice %s (%q) replaces manual reminder %q\n", n.ID, n.Heading, before.Message)
		}
	}
}
