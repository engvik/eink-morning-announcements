# Backend

The backend is a Go application that performs two things:

1) Is a HTTP server serving an API for the frontend and the ESP32.
2) Runs tasks to fetch forecasts and calendar events on a regular basis.

## Run locally

From repository root folder: `devenv up`

## Tasks

To write a new task, implement the following interface:

```go
    Name() string           // Should return the task name.
    Run(context.Context)    // Should implement the task runner itself. Note
                            // that tasks runs in it's own Go routine, so it
                            // must block otherwise the task will just exit.
                            // Return when the context is cancelled.
```

Then it can be passed into `tasks.Start(ctx, ...tasks)`

Tasks update once on startup and then on every tick, so a fresh process serves
real data immediately instead of waiting out the first interval.

## API

### GET /api/calendar

Get calendar events.

### GET /api/weather

Get weather forecasts.

### GET /api/message

Get latest message.

### POST /api/message

Create a new message.

Payload:

```json
{ "message": "my-message" }
```

### GET /api/meta

Get the current date, ISO week and a date-to-weekday map for the coming two
weeks.

## Storage

Storage is split by whether the data can be fetched again.

Calendar events and weather forecasts are a **cache**, both are re-fetchable
from the ICS feed and MET, so they are held in memory (`storage.Memory`) and
never persisted. Each fetch replaces the previous set, which is also what keeps
past entries from piling up.

The message is **not** a cache. It is written from the frontend and exists
nowhere else, so it is persisted in `sqlite` (`storage.SQLite`).

`storage.Client` composes the two and is what gets passed to `storage.New`. To
swap either half, implement the interface and compose your own:

```go
    SetCalendarEvents(context.Context, []calendar.Event) error
    GetCalendarEvents(context.Context) ([]calendar.Event, error)
    SetWeatherForecasts(context.Context, []weather.Forecast) error
    GetWeatherForecasts(context.Context) ([]weather.Forecast, error)
    SetMessage(context.Context, message.Message) error
    GetMessage(context.Context) (message.Message, error)
```

### Migrations

The schema is defined in `pkg/storage/migrations/*.sql`, embedded into the binary
and applied by `NewSQLiteClient` on open. `PRAGMA user_version` records how many
have run.

```
migrations/
  001_initial.sql
```

### Tables

#### Messages

```sql
CREATE TABLE messages (
  time INTEGER,
  message TEXT
);
```
