package calendar

import (
	"strings"
	"time"

	ics "github.com/arran4/golang-ical"

	"github.com/engvik/eink/backend/internal/config"
)

type Parser struct {
	Peek time.Duration
}

func NewParser(cfg *config.Config) *Parser {
	return &Parser{
		Peek: cfg.CalendarPeek,
	}
}

func (p *Parser) Parse(cal string) ([]Event, error) {
	calendar, err := ics.ParseCalendar(strings.NewReader(cal))
	if err != nil {
		return []Event{}, err
	}

	events := make([]Event, 0)
	now := time.Now()
	peek := now.Add(p.Peek)

	for _, e := range calendar.Events() {
		eStart, err := e.GetStartAt()
		if err != nil {
			return events, err
		}

		if eStart.After(now) && eStart.Before(peek) {
			eEnd, err := e.GetEndAt()
			if err != nil {
				return events, nil
			}

			summary := e.GetProperty(ics.ComponentProperty(ics.PropertySummary))
			description := e.GetProperty(ics.ComponentProperty(ics.PropertyDescription))
			location := e.GetProperty(ics.ComponentProperty(ics.PropertyLocation))

			events = append(events, Event{
				Start:       eStart,
				End:         eEnd,
				ID:          e.Id(),
				Title:       summary.Value,
				Description: description.Value,
				Location:    location.Value,
			})
		}
	}

	return events, nil
}
