package calendar

import (
	"log"
	"strings"
	"time"

	ics "github.com/arran4/golang-ical"
)

type Parser struct{}

func NewParser() *Parser {
	return &Parser{}

}

func (p *Parser) Parse(cal string) error {
	calendar, err := ics.ParseCalendar(strings.NewReader(cal))
	if err != nil {
		return err
	}

	events := calendar.Events()
	now := time.Now()
	for _, e := range events {
		et, err := e.GetStartAt()
		if err != nil {
			return err
		}

		if et.After(now) {
			log.Println(e.Serialize())
		}

	}

	return nil
}
