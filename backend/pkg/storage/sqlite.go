package storage

import (
	"context"
	"database/sql"
	"fmt"
	"time"

	_ "github.com/mattn/go-sqlite3"

	"github.com/engvik/eink-morning-announcements/backend/internal/config"
	"github.com/engvik/eink-morning-announcements/backend/pkg/calendar"
	"github.com/engvik/eink-morning-announcements/backend/pkg/message"
	"github.com/engvik/eink-morning-announcements/backend/pkg/weather"
)

type SQLite struct {
	db       *sql.DB
	location *time.Location
}

func NewSQLiteClient(cfg *config.Config) (*SQLite, error) {
	db, err := sql.Open("sqlite3", cfg.SQLitePath)
	if err != nil {
		return nil, err
	}

	location, err := time.LoadLocation(cfg.Location)
	if err != nil {
		return nil, err
	}

	return &SQLite{
		db:       db,
		location: location,
	}, nil
}

func (c *SQLite) SetCalendarEvents(ctx context.Context, events []calendar.Event) error {
	tx, err := c.db.BeginTx(ctx, nil)
	if err != nil {
		return err
	}

	stmt, err := tx.PrepareContext(
		ctx,
		`
		INSERT OR REPLACE INTO events
		VALUES (?, ?, ?, ?, ?, ?)
		`,
	)
	if err != nil {
		tx.Rollback()
		return err
	}

	for _, e := range events {
		if _, err := stmt.ExecContext(ctx, e.ID, e.Start.UnixMicro(), e.End.UnixMicro(), e.Title, e.Description, e.Location); err != nil {
			tx.Rollback()
			return err
		}
	}

	return tx.Commit()
}

func (c *SQLite) GetCalendarEvents(ctx context.Context) ([]calendar.Event, error) {
	now := time.Now()
	rows, err := c.db.QueryContext(
		ctx,
		`
		SELECT *
		FROM events
		WHERE start >= ?
		AND end <= ?
		ORDER BY start ASC
		LIMIT 3
		`,
		now.UnixMicro(),
		now.Add(time.Hour*24*3).UnixMicro(),
	)
	if err != nil {
		return []calendar.Event{}, err
	}

	defer rows.Close()

	events := make([]calendar.Event, 0, 10)

	for rows.Next() {
		var (
			id          string
			start       int64
			end         int64
			title       string
			description string
			location    string
		)

		if err := rows.Scan(&id, &start, &end, &title, &description, &location); err != nil {
			return events, fmt.Errorf("scan: %w", err)
		}

		events = append(
			events,
			calendar.Event{
				ID:          id,
				Start:       time.UnixMicro(start).In(c.location),
				End:         time.UnixMicro(end).In(c.location),
				Title:       title,
				Description: description,
				Location:    location,
			},
		)
	}

	if err := rows.Err(); err != nil {
		return events, err
	}

	return events, nil
}

func (c *SQLite) SetWeatherForecasts(ctx context.Context, forecasts []weather.Forecast) error {
	tx, err := c.db.BeginTx(ctx, nil)
	if err != nil {
		return err
	}

	stmt, err := tx.PrepareContext(
		ctx,
		`
		INSERT OR REPLACE INTO forecasts
		VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
		`,
	)
	if err != nil {
		tx.Rollback()
		return err
	}

	for _, f := range forecasts {
		if _, err := stmt.ExecContext(
			ctx,
			f.Time.UnixMicro(),
			f.Instant.AirPressureAtSeaLevel,
			f.Instant.AirTemperature,
			f.Instant.CloudAreaFraction,
			f.Instant.RelativeHumidity,
			f.Instant.WindFromDirection,
			f.Instant.WindSpeed,
			f.OneHour.SymbolCode,
			f.OneHour.PrecipitationAmount,
			f.SixHours.SymbolCode,
			f.SixHours.PrecipitationAmount,
			f.TwelveHours.SymbolCode,
		); err != nil {
			tx.Rollback()
			return err
		}
	}

	return tx.Commit()
}

func (c *SQLite) GetWeatherForecasts(ctx context.Context) ([]weather.Forecast, error) {
	rows, err := c.db.QueryContext(
		ctx,
		`
		SELECT *
		FROM forecasts
		WHERE time >= ?
		ORDER BY time ASC
		LIMIT 10
		`,
		time.Now().UnixMicro(),
	)
	if err != nil {
		return []weather.Forecast{}, err
	}

	defer rows.Close()

	forecasts := make([]weather.Forecast, 0, 10)

	for rows.Next() {
		var (
			timestamp                    int64
			instantAirPressureAtSeaLevel float64
			instantAirTemperature        float64
			instantCloudAreaFraction     float64
			instantRelativeHumidity      float64
			instantWindFromDirection     float64
			instantWindSpeed             float64
			oneHourSymbolCode            string
			oneHourPrecipitationAmount   float64
			sixHoursSymbolCode           string
			sixHoursPrecipitationAmount  float64
			twelveHoursSymbolCode        string
		)

		if err := rows.Scan(
			&timestamp,
			&instantAirPressureAtSeaLevel,
			&instantAirTemperature,
			&instantCloudAreaFraction,
			&instantRelativeHumidity,
			&instantWindFromDirection,
			&instantWindSpeed,
			&oneHourSymbolCode,
			&oneHourPrecipitationAmount,
			&sixHoursSymbolCode,
			&sixHoursPrecipitationAmount,
			&twelveHoursSymbolCode,
		); err != nil {
			return forecasts, fmt.Errorf("scan: %w", err)
		}

		forecasts = append(
			forecasts,
			weather.Forecast{
				Time: time.UnixMicro(timestamp).In(c.location),
				Instant: weather.InstantForecast{
					AirPressureAtSeaLevel: instantAirPressureAtSeaLevel,
					AirTemperature:        instantAirTemperature,
					CloudAreaFraction:     instantCloudAreaFraction,
					RelativeHumidity:      instantRelativeHumidity,
					WindFromDirection:     instantWindFromDirection,
					WindSpeed:             instantWindSpeed,
				},
				OneHour: weather.PeriodForecast{
					SymbolCode:          oneHourSymbolCode,
					PrecipitationAmount: oneHourPrecipitationAmount,
				},
				SixHours: weather.PeriodForecast{
					SymbolCode:          sixHoursSymbolCode,
					PrecipitationAmount: sixHoursPrecipitationAmount,
				},
				TwelveHours: weather.PeriodForecast{
					SymbolCode: twelveHoursSymbolCode,
				},
			},
		)
	}

	if err := rows.Err(); err != nil {
		return forecasts, err
	}

	return forecasts, nil
}

func (c *SQLite) SetMessage(ctx context.Context, m message.Message) error {
	stmt, err := c.db.PrepareContext(
		ctx,
		`
		INSERT OR REPLACE INTO messages
		VALUES (?, ?)
		`,
	)
	if err != nil {
		return err
	}

	if _, err := stmt.ExecContext(ctx, time.Now().UnixMicro(), m.Message); err != nil {
		return err
	}

	return nil
}

func (c *SQLite) GetMessage(ctx context.Context) (message.Message, error) {
	var timestamp int64
	var msg string

	err := c.db.QueryRowContext(ctx, "SELECT * FROM messages ORDER BY time DESC LIMIT 1").Scan(&timestamp, &msg)
	if err != nil {
		return message.Message{}, err
	}

	return message.Message{
		Time:    time.UnixMicro(timestamp).In(c.location),
		Message: msg,
	}, nil
}

func (c *SQLite) Close() {
	c.db.Close()
}
