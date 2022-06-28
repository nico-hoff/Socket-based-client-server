FROM alpine:3.15.4

COPY . /app

RUN apk update && apk add build-base gcc protobuf-dev

WORKDIR /app
RUN make


