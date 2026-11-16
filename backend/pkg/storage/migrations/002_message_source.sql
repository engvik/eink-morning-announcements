ALTER TABLE messages ADD COLUMN source TEXT NOT NULL DEFAULT 'manual';
ALTER TABLE messages ADD COLUMN external_id TEXT;
CREATE UNIQUE INDEX messages_source_external_id ON messages (source, external_id);
