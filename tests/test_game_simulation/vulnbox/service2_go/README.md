# Example Service 2

Service used go && mysql

[http://localhost:4442/](http://localhost:4442/)

## Install requriments

```
$ sudo apt install golang
$ go get github.com/go-sql-driver/mysql
$ go get github.com/jmoiron/sqlx
$ go get github.com/gorilla/mux
```

## Build

```
$ cd src
$ go build server.go
```

## Run

```
$ cd src
$ go run server.go
```

