/**********************************************************************************
 * MIT License
 *
 * Copyright (c) 2025-2026 Evgenii Sopov <mrseakg@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 *all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Official Source Code: https://github.com/sea5kg/sea5kg-sqlite3-wrapper
 *
 ***********************************************************************************/

#include "sea5kg_sqlite3_wrapper.h"
#include <sea5kg_logger.h>
#include <sqlite3.h>
#include <wsjcpp_core.h>
#include <wsjcpp_employees.h>

namespace sea5kg {

namespace sqlite3_wrapper {

database_update_info::database_update_info(
  const std::string &version_from, const std::string &version_to, const std::string &description
)
    : m_version_from(version_from), m_version_to(version_to), m_description(description) {
}

const std::string &database_update_info::version_from() const {
  return m_version_from;
}

const std::string &database_update_info::version_to() const {
  return m_version_to;
}

const std::string &database_update_info::description() const {
  return m_description;
}

std::map<std::string, std::vector<std::shared_ptr<database_update_fabric_base>>> *g_database_updates_fabric = nullptr;

// static
void global::registry_database_update_fabric(
  const std::string &db_name, std::shared_ptr<database_update_fabric_base> fab
) {
  if (g_database_updates_fabric == nullptr) {
    g_database_updates_fabric = new std::map<std::string, std::vector<std::shared_ptr<database_update_fabric_base>>>();
  }
  if (g_database_updates_fabric->count(db_name) == 0) {
    g_database_updates_fabric->insert(
      std::pair<std::string, std::vector<std::shared_ptr<database_update_fabric_base>>>(db_name, {})
    );
  }
  g_database_updates_fabric->at(db_name).push_back(fab);
}

std::map<std::string, database_file *> *g_opened_database_files = nullptr;

// static
void global::add_opened_database_file(const std::string &name, database_file *db) {
  if (g_opened_database_files == nullptr) {
    // sea5kg::log::info(std::string(), "Create employees map");
    g_opened_database_files = new std::map<std::string, database_file *>();
  }
  if (g_opened_database_files->find(name) != g_opened_database_files->end()) {
    // "Already registered '" + name + "'";
  } else {
    g_opened_database_files->insert(std::pair<std::string, database_file *>(name, db));
  }
}

// static
bool global::init_driver_sqlite3(int &ret) {
  ret = sqlite3_initialize();
  return SQLITE_OK == ret;
}

// static
void global::shutdown_driver_sqlite3() {
  // will be automatically closed all opened databases
  if (g_opened_database_files != nullptr) {
    for (const auto &pair : *g_opened_database_files) {
      pair.second->close();
    }
  }
  sqlite3_shutdown();
}

database_update::database_update(
  const std::string &db_name,
  const std::string &version_from,
  const std::string &version_to,
  const std::string &description
)
    : m_update_info(version_from, version_to, description), m_db_name(db_name) {
}

const database_update_info &database_update::info() {
  return m_update_info;
};

const std::string &database_update::db_name() const {
  return m_db_name;
};

void database_update::set_weight(int weight) {
  m_weight = weight;
}

int database_update::weight() {
  return m_weight;
}

class impl_rows_iterator : public rows_iterator {
public:
  impl_rows_iterator();
  ~impl_rows_iterator();
  void set_stmt(sqlite3_stmt *stmt);
  void *stmt();
  virtual bool next() override;
  virtual std::string as_string(int column_idx) override;
  virtual long as_long(int column_idx) override;

private:
  sqlite3_stmt *m_stmt;
};

impl_rows_iterator::impl_rows_iterator() {
  m_stmt = nullptr;
}

impl_rows_iterator::~impl_rows_iterator() {
  if (m_stmt != nullptr) {
    sqlite3_finalize(m_stmt);
  }
}

void impl_rows_iterator::set_stmt(sqlite3_stmt *stmt) {
  m_stmt = stmt;
}

void *impl_rows_iterator::stmt() {
  return m_stmt;
}

bool impl_rows_iterator::next() {
  return sqlite3_step(m_stmt) == SQLITE_ROW;
}

std::string impl_rows_iterator::as_string(int column_idx) {
  return std::string((const char *)sqlite3_column_text(m_stmt, column_idx));
}

long impl_rows_iterator::as_long(int column_idx) {
  return sqlite3_column_int64(m_stmt, column_idx);
}

// ---------------------------------------------------------------------
// database_file

database_file::database_file(
  const std::string &db_name,
  const std::string &init_sql,
  const std::string &db_dir,
  const std::string &filename,
  long backup_freq
)
    : m_db(nullptr), m_backup_freq_in_seconds(backup_freq), m_last_backup_time(0), m_initial_version("initial"),
      m_db_name(db_name) {
  TAG = "database_file-" + filename;
  m_filename = filename != "" ? filename : db_name + ".db";

  std::string sDatabaseDir = db_dir;
  m_filepath = sDatabaseDir + "/" + m_filename;

  if (m_backup_freq_in_seconds > 0) {
    std::string sDatabaseBackupDir = db_dir + "/backups";
    if (!wsjcpp::dir_exists(sDatabaseBackupDir)) {
      if (!WsjcppCore::makeDir(sDatabaseBackupDir)) {
        sea5kg::log::critical(TAG, "Could not create dir " + sDatabaseBackupDir);
      }
      std::string error;
      if (!WsjcppCore::setFilePermissions(sDatabaseBackupDir, WsjcppFilePermissions(0x776), error)) {
        sea5kg::log::critical(TAG, error);
      }
    }
    m_basename_backup_filepath = sDatabaseBackupDir + "/" + m_filename;
  }

  if (g_database_updates_fabric != nullptr) {
    if (g_database_updates_fabric->count(m_db_name)) {
      std::vector<std::shared_ptr<database_update_fabric_base>> fabs = g_database_updates_fabric->at(m_db_name);
      for (auto it : fabs) {
        // std::cout << "Found update" << std::endl;
        m_db_updates.push_back(it->create_update());
      }
    }
  }

  m_init_sql = init_sql;
  if (!wsjcpp::dir_exists(db_dir)) {
    sea5kg::log::critical(TAG, "Not found db dir: " + db_dir);
  }
};

database_file::~database_file() {
  close();
}

const std::string &database_file::filename() const {
  return m_filename;
}

const std::string &database_file::filepath() const {
  return m_filepath;
}

bool database_file::contains_table(const std::string &table_name) {
  sqlite3 *db = (sqlite3 *)m_db;
  std::string sSqlCheckVersionTable =
    "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='" + table_name + "';";
  std::string error;
  int cnt = select_sum_or_count(sSqlCheckVersionTable.c_str(), error);
  return cnt == 1;
}

bool database_file::open(std::string &error) {
  m_db = nullptr;
  // TODO if could not open but has backup try open backup
  sqlite3 *db = (sqlite3 *)m_db;
  int nRet = sqlite3_open_v2(m_filepath.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (nRet != SQLITE_OK) {
    error = "Failed to open database: " + std::to_string(nRet);
    m_db = nullptr;
    return false;
  }
  m_db = db;

  // Run the SQL
  if (m_init_sql != "") {
    if (!execute_query(m_init_sql, error)) {
      close();
      return false;
    }
  }
  if (!create_table_db_version(error)) {
    close();
    return false;
  }

  if (!copy_database_to_backup(error)) {
    close();
    return false;
  }

  if (!install_updates(error)) {
    close();
    return false;
  }

  sea5kg::sqlite3_wrapper::global::add_opened_database_file(m_filepath, this);
  return true;
}

bool database_file::is_opened() const {
  return m_db != nullptr;
}

void database_file::close() {
  if (is_opened()) {
    sqlite3_close((sqlite3 *)m_db);
    m_db = nullptr;
  }
}

bool database_file::execute_query(const std::string &sql, std::string &error) {
  if (!is_opened()) {
    error = "Database not opened";
    return false;
  }
  if (!copy_database_to_backup(error)) {
    return false;
  }
  char *errMsg = 0;
  sqlite3 *db = (sqlite3 *)m_db;
  int ret = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
  if (ret != SQLITE_OK) {
    error = "Problem with execute sql=query: '" + std::string(errMsg) + "'. Incoming SQL: " + sql;
    sqlite3_free(errMsg);
    return false;
  }
  return true;
}

int database_file::select_sum_or_count(const std::string &sql, std::string &error) {
  if (!is_opened()) {
    error = "Database not opened";
    return -1;
  }
  sqlite3 *db = (sqlite3 *)m_db;
  sqlite3_stmt *pQuery = nullptr;
  int ret = sqlite3_prepare_v2((sqlite3 *)m_db, sql.c_str(), -1, &pQuery, NULL);
  // prepare the statement
  if (ret != SQLITE_OK) {
    error = "Failed to prepare select count: " + std::string(sqlite3_errmsg((sqlite3 *)m_db)) + "\n SQL-query: " + sql;
    sea5kg::log::critical(TAG, error);
    return -1;
  }
  // step to 1st row of data
  ret = sqlite3_step(pQuery);
  if (ret != SQLITE_ROW) { // see documentation, this can return more values as success
    error = "Failed to step for select count or sum: " + std::string(sqlite3_errmsg((sqlite3 *)m_db)) +
            "\n SQL-query: " + sql;
    sea5kg::log::critical(TAG, error);
    return -1;
  }
  int nRet = sqlite3_column_int(pQuery, 0);
  if (pQuery != nullptr)
    sqlite3_finalize(pQuery);
  return nRet;
}

std::shared_ptr<rows_iterator> database_file::select_rows(const std::string &sql, std::string &error) {
  if (!is_opened()) {
    // error = "Database not opened";
    return nullptr;
  }
  sqlite3 *db = (sqlite3 *)m_db;
  sqlite3_stmt *stmt = nullptr;
  int res = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
  // prepare the statement
  if (res != SQLITE_OK) {
    error = "Failed to prepare select rows: '" + std::string(sqlite3_errmsg(db)) + "'. SQL: " + sql;
    return nullptr;
  }
  auto ret = std::make_shared<impl_rows_iterator>();
  ret->set_stmt(stmt);
  return ret;
}

bool database_file::install_updates(std::string &error) {
  if (!is_opened()) {
    error = "Database not opened";
    return false;
  }
  sqlite3 *db = (sqlite3 *)m_db;

  // Installed updates
  std::vector<database_update_info> installedUpdates;
  {
    sqlite3_stmt *pQuery = nullptr;
    const std::string sSqlCurrentVersion =
      "SELECT version_from, version_to, description FROM db_version ORDER BY rowid";
    int ret = sqlite3_prepare_v2(db, sSqlCurrentVersion.c_str(), -1, &pQuery, NULL);
    if (ret != SQLITE_OK) {
      error = "Failed to prepare: " + std::string(sqlite3_errmsg(db)) + "\n SQL-query: " + sSqlCurrentVersion;
      return false;
    }
    ret = sqlite3_step(pQuery);
    while (ret == SQLITE_ROW) {
      database_update_info info(
        std::string((const char *)sqlite3_column_text(pQuery, 0)), // from
        std::string((const char *)sqlite3_column_text(pQuery, 1)), // to
        std::string((const char *)sqlite3_column_text(pQuery, 2))  // decr
      );
      installedUpdates.push_back(info);
      ret = sqlite3_step(pQuery);
    }
    if (pQuery != nullptr)
      sqlite3_finalize(pQuery);
  }

  std::vector<std::string> installedVersionsTo;
  installedVersionsTo.push_back(m_initial_version);
  for (int i = 0; i < installedUpdates.size(); i++) {
    installedVersionsTo.push_back(installedUpdates[i].version_to());
  }

  // install updates
  bool bInstalledNewUpdates = true;
  while (bInstalledNewUpdates) {
    bInstalledNewUpdates = false;
    std::vector<std::string> installedNewUpdates;

    for (const auto &upd : m_db_updates) {
      const std::string &sVersionFrom = upd->info().version_from();
      const std::string &sVersionTo = upd->info().version_to();

      for (int iv = 0; iv < installedVersionsTo.size(); iv++) {
        if (sVersionFrom == installedVersionsTo[iv]) {
          auto it = std::find(installedVersionsTo.begin(), installedVersionsTo.end(), sVersionTo);
          if (it == installedVersionsTo.end()) {
            if (!upd->apply_update(this, error)) {
              error +=
                " (Could not install update " + upd->info().version_from() + " -> " + upd->info().version_to() + ")";
              return false;
            }
            if (!insert_db_version(upd->info(), error)) {
              return false;
            }
            installedNewUpdates.push_back(sVersionTo);
          } else {
            // skip update
          }
        }
      }
    }
    bInstalledNewUpdates = installedNewUpdates.size() > 0;
    for (int i = 0; i < installedNewUpdates.size(); i++) {
      installedVersionsTo.push_back(installedNewUpdates[i]);
    }
  }

  return true;
}

bool database_file::insert_db_version(const database_update_info &info, std::string &error) {
  if (!is_opened()) {
    error = "Database not opened";
    return false;
  }
  // TODO escaping
  long nCurrentTime =
    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  std::string sql = "INSERT INTO "
                    "db_version(version_from, version_to, description, dt) "
                    "VALUES(\"" +
                    info.version_from() + "\", \"" + info.version_to() + "\", \"" + info.description() + "\", " +
                    std::to_string(nCurrentTime) + ")";
  if (!this->execute_query(sql, error)) {
    return false;
  }
  return true;
}

bool database_file::copy_database_to_backup(std::string &error) {
  if (m_backup_freq_in_seconds <= 0) {
    return true;
  }

  std::lock_guard<std::mutex> lock(m_mutex);
  long nCurrentTime =
    std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  if (nCurrentTime - m_last_backup_time < m_backup_freq_in_seconds) {
    return true;
  }
  m_last_backup_time = nCurrentTime;

  int nMaxBackupsFiles = 9;
  sea5kg::log::info(TAG, "Start backup for " + m_filepath);
  std::string sFilebackup = m_basename_backup_filepath + "." + std::to_string(nMaxBackupsFiles);
  if (wsjcpp::file_exists(sFilebackup)) {
    WsjcppCore::removeFile(sFilebackup);
  }
  for (int i = nMaxBackupsFiles - 1; i >= 0; i--) {
    std::string sFilebackupFrom = m_basename_backup_filepath + "." + std::to_string(i);
    std::string sFilebackupTo = m_basename_backup_filepath + "." + std::to_string(i + 1);
    if (wsjcpp::file_exists(sFilebackupFrom)) {
      if (std::rename(sFilebackupFrom.c_str(), sFilebackupTo.c_str())) {
        sea5kg::log::critical(TAG, "Could not rename from " + sFilebackupFrom + " to " + sFilebackupTo);
        return false;
      }
    }
  }
  sFilebackup = m_basename_backup_filepath + "." + std::to_string(0);
  if (!WsjcppCore::copyFile(m_filepath, sFilebackup)) {
    sea5kg::log::critical(TAG, "Failed copy file to backup for " + m_filepath);
  }
  sea5kg::log::info(TAG, "Backup done for " + m_filepath);
  return true;
}

bool database_file::create_table_db_version(std::string &error) {
  if (!is_opened()) {
    error = "Database not opened";
    return false;
  }
  if (!contains_table("db_version")) {
    // create db_version
    sqlite3 *db = (sqlite3 *)m_db;
    const std::string sql = "CREATE TABLE IF NOT EXISTS db_version ( "
                            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "  version_from VARCHAR(64),"
                            "  version_to VARCHAR(64),"
                            "  dt INTEGER NOT NULL,"
                            "  description VARCHAR(2048) NOT NULL"
                            ");";
    char *zErrMsg = 0;
    int nRet = sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg);
    if (nRet != SQLITE_OK) {
      error = "Problem with create table: " + std::string(zErrMsg);
      return false;
    }
  }
  return true;
}

} // namespace sqlite3_wrapper

} // namespace sea5kg
