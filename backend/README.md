# Backend

## Database

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
