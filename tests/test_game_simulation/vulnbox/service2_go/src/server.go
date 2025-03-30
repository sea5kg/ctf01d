package main

import (
  "fmt"
	"net/http"
  "github.com/gorilla/mux"
  "html/template"
  _ "github.com/go-sql-driver/mysql"
  "github.com/jmoiron/sqlx"
  "encoding/json"
  "os"
)

func getConnString() string {
  var sRet string
  sRet = os.Getenv("SERVICE2_GO_MYSQL_USER") + ":"
  sRet = sRet + os.Getenv("SERVICE2_GO_MYSQL_PASSWORD")
  sRet = sRet + "@tcp(" + os.Getenv("SERVICE2_GO_MYSQL_HOST") + ":3306)/"
  sRet = sRet + os.Getenv("SERVICE2_GO_MYSQL_DBNAME")
  // fmt.Printf("getConnString sRet=%s\n", sRet)
  return sRet
}
      
type Flag struct {
  Flag_id string `db:"flag_id"`
  Flag string `db:"flag"`
}

type ErrorJson struct {
  Error string
}

// --------------------------------------------------------

func err_json(w http.ResponseWriter, msg string) {
  errorJson := ErrorJson{msg}
  js, err := json.Marshal(errorJson)
  if err != nil {
    http.Error(w, err.Error(), http.StatusInternalServerError)
    return
  }
  
  w.Header().Set("Content-Type", "application/json")
  w.WriteHeader(400)
  w.Write(js)
}

// --------------------------------------------------------

func put_flag(w http.ResponseWriter, r *http.Request) {
  params := mux.Vars(r)
  flag_id := params["flag_id"]
  flag := params["flag"]

  db, err := sqlx.Connect("mysql", getConnString())
  if err != nil {
    fmt.Printf("FAILED connect to database %s\n", err.Error())
    err_json(w, err.Error())
    return
  }

  _, err2 := db.Exec(`INSERT INTO flag (flag_id, flag) VALUES(?,?)`, flag_id, flag)
	if err2 != nil {
    fmt.Printf("FAILED insert flag=%s with flag_id=%s\n", flag_id, flag)
    err_json(w, err2.Error())
    db.Close()
    return
  }
  flag2 := Flag{flag_id, flag}
  js, err := json.Marshal(flag2)
  if err != nil {
    http.Error(w, err.Error(), http.StatusInternalServerError)
    db.Close()
    return
  }
  
  fmt.Printf("Inserted flag=%s with flag_id=%s\n", flag_id, flag)
  w.Header().Set("Content-Type", "application/json")
  w.Write(js)
  db.Close()
}

// --------------------------------------------------------

func get_flag(w http.ResponseWriter, r *http.Request) {
  params := mux.Vars(r)
  flag_id := params["flag_id"]
  
  db, err := sqlx.Connect("mysql", getConnString())
  if err != nil {
    fmt.Printf("FAILED connect to database %s\n", err.Error())
    err_json(w, err.Error())
    return
  }

  var flag Flag
  err2 := db.Get(&flag, `SELECT * FROM flag WHERE flag_id = ?`, flag_id)
	if err2 != nil {
    db.Close()
    err_json(w, err2.Error())
    return
  }

  js, err := json.Marshal(flag)
  if err != nil {
    http.Error(w, err.Error(), http.StatusInternalServerError)
    return
  }
  fmt.Printf("Get flag=%s with flag_id=%s", flag.Flag_id, flag.Flag)

  w.Header().Set("Content-Type", "application/json")
  w.Write(js)
  db.Close()
}

// --------------------------------------------------------

type FlagIDs struct {
  FlagIDs []string
}

func list_flag(w http.ResponseWriter, r *http.Request) {
  db, err := sqlx.Connect("mysql", getConnString())
  if err != nil {
    fmt.Printf("FAILED connect to database %s\n", err.Error())
    err_json(w, err.Error())
    return
  }

  var flag_ids []string
  err2 := db.Select(&flag_ids, "SELECT flag_id FROM flag")
  if err2 != nil {
    err_json(w, err2.Error())
    db.Close()
    return
  }
  flags2 := FlagIDs{flag_ids}
  js, err := json.Marshal(flags2)
  if err != nil {
    http.Error(w, err.Error(), http.StatusInternalServerError)
    db.Close()
    return
  }
  
  w.Header().Set("Content-Type", "application/json")
  w.Write(js)
  db.Close()
}

// --------------------------------------------------------

func index(w http.ResponseWriter, r *http.Request) {
  t, _ := template.ParseFiles("html/index.html")
  t.Execute(w, nil)
}

// --------------------------------------------------------

func main() {
  fmt.Printf("Start server...\n")
  rtr := mux.NewRouter()
  rtr.HandleFunc("/", index).Methods("GET")
  rtr.HandleFunc("/api/flags/{flag_id:[a-zA-Z0-9]+}", get_flag).Methods("GET")
  rtr.HandleFunc("/api/flags/{flag_id:[a-zA-Z0-9]+}/{flag:[a-zA-Z0-9-]+}", put_flag).Methods("POST")
  rtr.HandleFunc("/api/flags/", list_flag)
  http.Handle("/", rtr)

  fs := http.FileServer(http.Dir("html/static"))
  http.Handle("/static/", http.StripPrefix("/static/", fs))

	http.ListenAndServe(":4102", nil)
}
