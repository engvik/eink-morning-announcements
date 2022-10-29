package meta

import "time"

type Meta struct {
	Today         string            `json:"today"`
	DateToWeekday map[string]string `json:"date_to_weekday"`
	Now           time.Time         `json:"now"`
}

func GetMeta() Meta {
	now := time.Now()

	return Meta{
		Today:         now.Weekday().String(),
		DateToWeekday: makeDateToWeekdayMap(now),
		Now:           now,
	}
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
