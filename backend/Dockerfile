FROM golang:1.25-bookworm AS builder

WORKDIR /app

COPY . ./

RUN go mod download

RUN CGO_ENABLED=0 GOOS=linux go build -mod=readonly -v -o /build/server cmd/server/main.go

#--

FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y tzdata ca-certificates && rm -rf /var/lib/apt/lists/*

COPY --from=builder /build/server /app/server

CMD ["/app/server"]
