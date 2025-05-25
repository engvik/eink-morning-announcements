FROM golang:1.24.0-bullseye AS builder

WORKDIR /app

COPY . ./

RUN go mod download

RUN CGO_ENABLED=1 GOOS=linux go build -mod=readonly -v -o /build/server cmd/server/main.go

#--

FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y sqlite3 tzdata

COPY --from=builder /build/server /app/server

CMD ["/app/server"]
