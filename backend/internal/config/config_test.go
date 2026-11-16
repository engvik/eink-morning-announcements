package config

import (
	"slices"
	"testing"
	"time"
)

func TestClocksDecode(t *testing.T) {
	tests := []struct {
		in      string
		want    Clocks
		wantErr bool
	}{
		{in: "07:25,15:00", want: Clocks{{7, 25}, {15, 0}}},
		{in: " 7:05 , ", want: Clocks{{7, 5}}},
		{in: "", wantErr: true},
		{in: "25:00", wantErr: true},
		{in: "0725", wantErr: true},
	}

	for _, tt := range tests {
		t.Run(tt.in, func(t *testing.T) {
			var got Clocks

			err := got.Decode(tt.in)
			if (err != nil) != tt.wantErr {
				t.Fatalf("Decode(%q) err = %v, wantErr %v", tt.in, err, tt.wantErr)
			}

			if !tt.wantErr && !slices.Equal(got, tt.want) {
				t.Errorf("Decode(%q) = %v, want %v", tt.in, got, tt.want)
			}
		})
	}
}

func TestClocksNext(t *testing.T) {
	oslo, err := time.LoadLocation("Europe/Oslo")
	if err != nil {
		t.Fatal(err)
	}

	at := func(y int, m time.Month, d, h, min int) time.Time { return time.Date(y, m, d, h, min, 0, 0, oslo) }
	clocks := Clocks{{15, 0}, {7, 25}}

	tests := []struct {
		name string
		now  time.Time
		want time.Time
	}{
		{"before first", at(2026, 11, 14, 6, 0), at(2026, 11, 14, 7, 25)},
		{"between", at(2026, 11, 14, 9, 0), at(2026, 11, 14, 15, 0)},
		{"exactly at a time moves on", at(2026, 11, 14, 7, 25), at(2026, 11, 14, 15, 0)},
		{"after last rolls to tomorrow", at(2026, 11, 14, 16, 0), at(2026, 11, 15, 7, 25)},
		{"month end", at(2026, 11, 30, 16, 0), at(2026, 12, 1, 7, 25)},
		{"across DST start", at(2027, 3, 27, 16, 0), at(2027, 3, 28, 7, 25)},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := clocks.Next(tt.now); !got.Equal(tt.want) {
				t.Errorf("Next(%v) = %v, want %v", tt.now, got, tt.want)
			}
		})
	}
}

func TestExcludesDecodeAndTitles(t *testing.T) {
	var e Excludes
	if err := e.Decode("Yoga, mykid=Første måltid , mykid=Tur,personal=Dugnad"); err != nil {
		t.Fatal(err)
	}

	want := Excludes{
		{Title: "Yoga"},
		{Feed: "mykid", Title: "Første måltid"},
		{Feed: "mykid", Title: "Tur"},
		{Feed: "personal", Title: "Dugnad"},
	}
	if !slices.Equal(e, want) {
		t.Fatalf("Decode = %+v, want %+v", e, want)
	}

	tests := []struct {
		label string
		want  []string
	}{
		{"mykid", []string{"Yoga", "Første måltid", "Tur"}},
		{"MyKid", []string{"Yoga", "Første måltid", "Tur"}}, // label match is case-insensitive
		{"personal", []string{"Yoga", "Dugnad"}},
		{"other", []string{"Yoga"}},
	}

	for _, tt := range tests {
		t.Run(tt.label, func(t *testing.T) {
			if got := e.Titles(tt.label); !slices.Equal(got, tt.want) {
				t.Errorf("Titles(%q) = %v, want %v", tt.label, got, tt.want)
			}
		})
	}
}
