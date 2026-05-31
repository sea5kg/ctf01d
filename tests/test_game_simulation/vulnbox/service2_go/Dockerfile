FROM golang:1.15-buster
LABEL "maintainer"="Evgenii Sopov <mrseakg@gmail.com>"
LABEL "repository"="https://github.com/sea-kg/ctf01d"

WORKDIR /go/src/app

COPY ./src/ /go/src/app

# Better use a localfolders
RUN go mod init service2_go \
    && go get github.com/go-sql-driver/mysql@v1.6.0 \
    && go get github.com/jmoiron/sqlx@v1.3.5 \
    && go get github.com/gorilla/mux@v1.8.0

EXPOSE 4102

CMD exec go run server.go

# CMD ["go","run","server.go"]

