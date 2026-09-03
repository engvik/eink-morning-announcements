package calendar

import (
	"log"
	"strings"
	"time"

	ics "github.com/arran4/golang-ical"
	"github.com/stephens2424/rrule"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
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

		// Find recurring events
		if eStart.Before(now) {
			rule := e.GetProperty(ics.ComponentProperty(ics.PropertyRrule))
			if rule != nil {
				parsed, err := rrule.ParseRRule(rule.Value)
				if err != nil {
					log.Println("Unable to parse RRULE:", err)
					continue
				}

				parsed.Dtstart = eStart // Set start date to recurring event start

				// Iterate until there are an event in the future and add it if
				// it's withing the  peek period.
				it := parsed.Iterator()
				var nextEvent *time.Time
				for {
					next := it.Next()
					if next == nil {
						break
					}

					if next.After(now) && next.Before(peek) {
						nextEvent = next
						break
					} else if next.After(now) {
						break
					}
				}

				if nextEvent != nil {
					events = append(events, createEvent(*nextEvent, e))
				}
			}
		}

		// Find upcoming events
		if eStart.After(now) && eStart.Before(peek) {
			events = append(events, createEvent(eStart, e))
		}
	}

	return events, nil
}

func createEvent(eStart time.Time, e *ics.VEvent) Event {
	// An event with neither DTEND nor DURATION is zero-length per RFC 5545.
	eEnd, err := e.GetEndAt()
	if err != nil {
		eEnd = eStart
	}

	summaryProperty := e.GetProperty(ics.ComponentProperty(ics.PropertySummary))
	descriptionProperty := e.GetProperty(ics.ComponentProperty(ics.PropertyDescription))
	locationProperty := e.GetProperty(ics.ComponentProperty(ics.PropertyLocation))

	return Event{
		Start:       eStart,
		End:         eEnd,
		ID:          e.Id(),
		Title:       getPropertyString(summaryProperty),
		Description: getPropertyString(descriptionProperty),
		Location:    getPropertyString(locationProperty),
	}
}

func getPropertyString(p *ics.IANAProperty) string {
	if p == nil {
		return ""
	}

	return p.Value
}
