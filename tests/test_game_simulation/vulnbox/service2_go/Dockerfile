FROM golang:1.15-buster
LABEL "maintainer"="Evgenii Sopov <mrseakg@gmail.com>"
LABEL "repository"="https://github.com/sea-kg/ctf01d"

WORKDIR /go/src/app

COPY ./src/ /go/src/app

# Better use a localfolders
RUN go get github.com/go-sql-driver/mysql
RUN go get github.com/jmoiron/sqlx
RUN go get github.com/gorilla/mux

EXPOSE 4202

CMD exec go run server.go

# CMD ["go","run","server.go"]


