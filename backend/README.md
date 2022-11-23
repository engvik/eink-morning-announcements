# Backend

The backend is a Go application that performs two things:

1) Is a HTTP server serving an API for the frontend and the ESP32.
2) Runs tasks to fetch forecasts and calendar events on a regular basis.

## Run locally

From repository root folder: `task run-backend`

## Tasks

To write a new task, implement the following interface:

```go
    Name() string       // Should return the task name.
    Run()               // Should implement the task runner itself. Note that
                        // tasks runs in it's own Go routine, so it must
                        // block otherwise the task will just exit.
```

Then it can be passed into `tasks.Start(...tasks)`

## API

### GET /api/calendar

Get calendar events.

### GET /api/weather

Get weather forecasts.

### GET /api/messages

Get latest message.

### POST /api/messages

Create a new message.

Payload:

```json
{ "message": "my-message" }
```

## Database

The backend uses a `sqlite` database as it should be enough for this
application, but hopefully it should be pretty easy to write support for
antoher database.

As long as it fulfills the interface,

```go
    SetCalendarEvents(context.Context, []calendar.Event) error
    GetCalendarEvents(context.Context) ([]calendar.Event, error)
    SetWeatherForecasts(context.Context, []weather.Forecast) error
    GetWeatherForecasts(context.Context) ([]weather.Forecast, error
    SetMessage(context.Context, message.Message) error
    GetMessage(context.Context) (message.Message, error)
```

it can be passed into `storage.New(myDBClientImplementation)`, and it should
just work.

### Tables

#### Calendar events

```sql
CREATE TABLE events (
   id TEXT PRIMARY KEY NOT NULL,
   start INTEGER,
   end INTEGER,
   title TEXT,
   description TEXT,
   location TEXT
);
```


#### Weather forecasts

```sql
CREATE TABLE forecasts (
  time INTEGER PRIMARY KEY NOT NULL,
  instant_air_pressure_at_sea_level REAL,
  instant_air_temperature REAL,
  instant_cloud_area_fraction REAL,
  instant_relative_humidity REAL,
  instant_wind_from_direction REAL,
  instant_wind_speed REAL,
  one_hour_symbol_code TEXT,
  one_hour_precipitation_amount REAL,
  six_hours_symbol_code TEXT,
  six_hours_precipitation_amount REAL,
  twelve_hours_symbol_code TEXT
);
```

#### Messages

```sql
CREATE TABLE messages (
  time INTEGER,
  message TEXT
);
```
