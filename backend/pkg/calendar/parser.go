package calendar

import (
	"log"
	"strings"
	"time"

	ics "github.com/arran4/golang-ical"
	"github.com/teambition/rrule-go"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
)

// The date and date-time forms iCalendar uses for EXDATE and RECURRENCE-ID.
const (
	icalDateTimeLayout = "20060102T150405"
	icalDateLayout     = "20060102"
)

type Parser struct {
	Peek time.Duration
}

func NewParser(cfg *config.Config) *Parser {
	return &Parser{
		Peek: cfg.CalendarPeek,
	}
}

func (p *Parser) Parse(cal string) (Events, error) {
	calendar, err := ics.ParseCalendar(strings.NewReader(cal))
	if err != nil {
		return Events{}, err
	}

	events := make(Events, 0)
	now := time.Now()
	peek := now.Add(p.Peek)

	overrides := collectOverrides(calendar.Events())

	for _, e := range calendar.Events() {
		eStart, err := e.GetStartAt()
		if err != nil {
			return events, err
		}

		// A recurring event stands for every occurrence in the window, not just
		// the first. Its DTSTART is the first of them, so it is not emitted
		// separately.
		if e.GetProperty(ics.ComponentPropertyRrule) != nil {
			events = append(events, expand(e, eStart, now, peek, overrides[e.Id()])...)
			continue
		}

		// Find events that have not ended yet.
		if eStart.Before(peek) {
			event := createEvent(eStart, e)
			if event.End.After(now) {
				events = append(events, event)
			}
		}
	}

	return events, nil
}

// expand walks a recurring event and returns every occurrence that has not
// ended and starts before the peek horizon. Slots listed in EXDATE, or replaced
// by a RECURRENCE-ID event, are left out.
func expand(e *ics.VEvent, eStart, now, peek time.Time, replaced map[time.Time]bool) Events {
	// RFC 5545 allows at most one RRULE per event, which is also all a Set
	// holds: adding a second would replace the first.
	rule, err := rrule.StrToRRule(e.GetProperty(ics.ComponentPropertyRrule).Value)
	if err != nil {
		log.Println("Unable to parse RRULE:", err)
		return nil
	}

	set := rrule.Set{}
	set.DTStart(eStart)
	set.RRule(rule)

	for _, prop := range e.GetProperties(ics.ComponentPropertyExdate) {
		// A single EXDATE property may carry several comma separated dates.
		for _, value := range strings.Split(prop.Value, ",") {
			excluded, err := parseICalTime(value, eStart.Location())
			if err != nil {
				log.Println("Unable to parse EXDATE:", err)
				continue
			}

			set.ExDate(excluded)
		}
	}

	// Set event length
	base := createEvent(eStart, e)
	duration := base.End.Sub(base.Start)

	// Expand the query window backward by one duration so we also catch
	// events that started before "now" but haven't ended yet (still ongoing).
	occurrences := set.Between(now.Add(-duration), peek, true)
	events := make(Events, 0, len(occurrences))

	for _, start := range occurrences {
		if replaced[start.UTC()] {
			continue
		}

		end := start.Add(duration)
		if !end.After(now) {
			continue
		}

		occurrence := base
		occurrence.Start = start
		occurrence.End = end

		events = append(events, occurrence)
	}

	return events
}

// collectOverrides builds, for each recurring event's UID, the set of
// original occurrence times that have been superseded by a modified
// instance (a separate VEVENT with a matching RECURRENCE-ID). The RRULE
// expansion later uses this set to skip those slots, since the modified
// instance is already included as its own event with its own DTSTART.
func collectOverrides(events []*ics.VEvent) map[string]map[time.Time]bool {
	overrides := make(map[string]map[time.Time]bool)

	for _, e := range events {
		prop := e.GetProperty(ics.ComponentPropertyRecurrenceId)
		if prop == nil {
			continue
		}

		replaced, err := parseICalTime(prop.Value, time.Local)
		if err != nil {
			log.Println("Unable to parse RECURRENCE-ID:", err)
			continue
		}

		if overrides[e.Id()] == nil {
			overrides[e.Id()] = make(map[time.Time]bool)
		}

		overrides[e.Id()][replaced.UTC()] = true
	}

	return overrides
}

// parseICalTime reads the date and date-time forms iCalendar uses for EXDATE
// and RECURRENCE-ID. A trailing Z means UTC, otherwise loc applies.
func parseICalTime(value string, loc *time.Location) (time.Time, error) {
	if utc, ok := strings.CutSuffix(value, "Z"); ok {
		return time.ParseInLocation(icalDateTimeLayout, utc, time.UTC)
	}

	if strings.Contains(value, "T") {
		return time.ParseInLocation(icalDateTimeLayout, value, loc)
	}

	return time.ParseInLocation(icalDateLayout, value, loc)
}

func createEvent(eStart time.Time, e *ics.VEvent) Event {
	allDay := isAllDay(e)

	eEnd, err := e.GetEndAt()
	if err != nil {
		// Without DTEND or DURATION an event is zero-length, except an all-day
		// one, which lasts a day. RFC 5545.
		eEnd = eStart
		if allDay {
			eEnd = eStart.AddDate(0, 0, 1)
		}
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
		AllDay:      allDay,
	}
}

// isAllDay reports whether DTSTART is a date rather than a timestamp, which is
// how iCalendar marks an event as covering whole days.
func isAllDay(e *ics.VEvent) bool {
	p := e.GetProperty(ics.ComponentPropertyDtStart)

	return p != nil && p.GetValueType() == ics.ValueDataTypeDate
}

func getPropertyString(p *ics.IANAProperty) string {
	if p == nil {
		return ""
	}

	return p.Value
}
