package meta

import "time"

type Meta struct {
	Today         string            `json:"today"`
	DateToWeekday map[string]string `json:"date_to_weekday"`
	Now           time.Time         `json:"now"`
	Date          int               `json:"date"`
	Week          int               `json:"week"`
	Month         string            `json:"month"`
	Year          int               `json:"year"`
}

func GetMeta(location string) (Meta, error) {
	now := time.Now()
	year, month, date := now.Date()
	_, week := now.ISOWeek()

	if location != "" {
		location, err := time.LoadLocation("Europe/Oslo")
		if err != nil {
			return Meta{}, err
		}

		now = now.In(location)
	}

	return Meta{
		Today:         now.Weekday().String(),
		DateToWeekday: makeDateToWeekdayMap(now),
		Now:           now,
		Date:          date,
		Week:          week,
		Month:         month.String(),
		Year:          year,
	}, nil
}

func makeDateToWeekdayMap(now time.Time) map[string]string {
	dateToWeekday := make(map[string]string, 0)
	t := now

	// Create map for the upcoming week
	for i := 0; i < 7; i++ {
		ft := t.Format("2006-01-02")
		dateToWeekday[ft] = t.Weekday().String()
		t = t.AddDate(0, 0, 1)
	}

	return dateToWeekday
}
