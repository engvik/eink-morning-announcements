FROM golang:1.24.0-bullseye AS builder

WORKDIR /app

COPY . ./

RUN go mod download

ENV CGO_ENABLED=0
RUN go build -mod=readonly -v -o /build/server cmd/server/main.go

#--

FROM gcr.io/distroless/static:nonroot

COPY --from=builder /build/server /app/server

CMD ["/app/server"]
