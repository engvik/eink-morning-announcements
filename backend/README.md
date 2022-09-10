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
