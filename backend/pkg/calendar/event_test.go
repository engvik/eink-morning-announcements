package calendar

import (
	"slices"
	"testing"
	"time"
)

func TestEventsDedupe(t *testing.T) {
	start := time.Date(2026, 11, 16, 10, 0, 0, 0, time.UTC)
	end := start.Add(time.Hour)

	tests := []struct {
		name string
		in   Events
		want []string
	}{
		{
			name: "empty",
			in:   Events{},
			want: []string{},
		},
		{
			name: "same title and times collapse to the first",
			in: Events{
				{ID: "kid1_tur", Title: "Tur", Start: start, End: end},
				{ID: "kid2_tur", Title: "Tur", Start: start, End: end},
			},
			want: []string{"kid1_tur"},
		},
		{
			name: "same instant in another location is a duplicate",
			in: Events{
				{ID: "a", Title: "Tur", Start: start, End: end},
				{ID: "b", Title: "Tur", Start: start.In(time.FixedZone("CET", 3600)), End: end},
			},
			want: []string{"a"},
		},
		{
			name: "different times are kept",
			in: Events{
				{ID: "s_kid1_tur", Title: "Foreldresamtale", Start: start, End: end},
				{ID: "s_kid2_tur", Title: "Foreldresamtale", Start: end, End: end.Add(time.Hour)},
			},
			want: []string{"s_kid1_tur", "s_kid2_tur"},
		},
		{
			name: "different titles are kept",
			in: Events{
				{ID: "a", Title: "Tur", Start: start, End: end},
				{ID: "b", Title: "Yoga", Start: start, End: end},
			},
			want: []string{"a", "b"},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := ids(tt.in.Dedupe()); !slices.Equal(got, tt.want) {
				t.Errorf("Dedupe() = %v, want %v", got, tt.want)
			}
		})
	}
}

func TestEventsExclude(t *testing.T) {
	in := Events{
		{ID: "1", Title: "Første måltid"},
		{ID: "2", Title: "  SAMLINGSTUND "},
		{ID: "3", Title: "Ha med-dag"},
		{ID: "4", Title: "Turdag! (Husk 2 matpakker)"},
	}

	tests := []struct {
		name   string
		titles []string
		want   []string
	}{
		{
			name:   "nil keeps all",
			titles: nil,
			want:   []string{"1", "2", "3", "4"},
		},
		{
			name:   "case and space insensitive",
			titles: []string{"første MÅLTID", "samlingstund "},
			want:   []string{"3", "4"},
		},
		{
			name:   "exact match only",
			titles: []string{"Tur", "Ha med"},
			want:   []string{"1", "2", "3", "4"},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := ids(in.Exclude(tt.titles)); !slices.Equal(got, tt.want) {
				t.Errorf("Exclude(%q) = %v, want %v", tt.titles, got, tt.want)
			}
		})
	}
}

func ids(e Events) []string {
	out := make([]string, 0, len(e))
	for _, ev := range e {
		out = append(out, ev.ID)
	}

	return out
}
