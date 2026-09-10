/**********************************************************************************
 *           Project
 *   _______ _________ _______  _______  __    ______
 *  (  ____ \\__   __/(  ____ \(  __   )/  \  (  __  \
 *  | (    \/   ) (   | (    \/| (  )  |\/) ) | (  \  )
 *  | |         | |   | (__    | | /   |  | | | |   ) |
 *  | |         | |   |  __)   | (/ /) |  | | | |   | |
 *  | |         | |   | (      |   / | |  | | | |   ) |
 *  | (____/\   | |   | )      |  (__) |__) (_| (__/  )
 *  (_______/   )_(   |/       (_______)\____/(______/
 *
 * MIT License
 *
 * Copyright (c) 2018-2026 Evgenii Sopov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Original repository: https://github.com/sea5kg/ctf01d
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

std::map<std::string, database_file *> *g_opened_database_files = nullptr;

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

// static
void global::add_opened_database_file(const std::string &name, database_file *db) {
  if (g_opened_database_files == nullptr) {
    // sea5kg::log::info(std::string(), "Create employees map");
    g_opened_database_files = new std::map<std::string, database_file *>();
  }
  if (g_opened_database_files->find(name) != g_opened_database_files->end()) {
    sea5kg::log::critical("WsjcppEmployees::addService", "Already registered '" + name + "'");
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

class impl_rows_iterator : public rows_iterator {
public:
  impl_rows_iterator();
  ~impl_rows_iterator();
  void set_stmt(sqlite3_stmt *pQuery);
  void *stmt();
  virtual bool next() override;
  virtual std::string as_string(int column_idx) override;
  virtual long as_long(int column_idx) override;

private:
  sqlite3_stmt *m_pQuery;
};

impl_rows_iterator::impl_rows_iterator() {
  m_pQuery = nullptr;
}

impl_rows_iterator::~impl_rows_iterator() {
  if (m_pQuery != nullptr) {
    sqlite3_finalize(m_pQuery);
  }
}

void impl_rows_iterator::set_stmt(sqlite3_stmt *pQuery) {
  m_pQuery = pQuery;
}

void *impl_rows_iterator::stmt() {
  return m_pQuery;
}

bool impl_rows_iterator::next() {
  return sqlite3_step(m_pQuery) == SQLITE_ROW;
}

std::string impl_rows_iterator::as_string(int column_idx) {
  return std::string((const char *)sqlite3_column_text(m_pQuery, column_idx));
}

long impl_rows_iterator::as_long(int column_idx) {
  return sqlite3_column_int64(m_pQuery, column_idx);
}

// ---------------------------------------------------------------------
// database_file

database_file::database_file(const std::string &db_name, const std::string &init_sql, const std::string &db_dir,
                             const std::string &filename, long backup_freq)
    : m_backup_freq_in_seconds(backup_freq) {
  TAG = "database_file-" + filename;
  m_db = nullptr;
  m_sFilename = filename;
  m_last_backup_time = 0;
  m_init_sql = init_sql;
  // auto config = findWsjcppEmploy<ctf01d::config>();
  if (!wsjcpp::dir_exists(db_dir)) {
    sea5kg::log::critical(TAG, "Not found db dir: " + db_dir);
  }
  m_sFileFullpath = db_dir + "/" + m_sFilename;

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
  m_sBaseFileBackupFullpath = sDatabaseBackupDir + "/" + m_sFilename;
};

database_file::~database_file() {
  close();
}

bool database_file::open(std::string &error) {
  m_db = nullptr;
  // TODO if could not open but has backup try open backup
  sqlite3 *db = (sqlite3 *)m_db;
  int nRet = sqlite3_open_v2(m_sFileFullpath.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (nRet != SQLITE_OK) {
    error = "Failed to open conn: " + std::to_string(nRet);
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
  sea5kg::log::success(TAG, "Opened database file " + m_sFileFullpath);
  if (!copy_database_to_backup(error)) {
    close();
    return false;
  }
  sea5kg::sqlite3_wrapper::global::add_opened_database_file(m_sFileFullpath, this);
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
  if (!copy_database_to_backup(error)) {
    return false;
  }
  char *errMsg = 0;
  int nRet = sqlite3_exec((sqlite3 *)m_db, sql.c_str(), 0, 0, &errMsg);
  if (nRet != SQLITE_OK) {
    error = "Problem with SQL: " + std::string(errMsg) + "\n SQL-query: " + sql;
    sqlite3_free(errMsg);
    return false;
  }
  return true;
}

int database_file::select_sum_or_count(const std::string &sql, std::string &error) {
  if (!copy_database_to_backup(error)) {
    return -1;
  }
  sqlite3_stmt *pQuery = nullptr;
  int ret = sqlite3_prepare_v2((sqlite3 *)m_db, sql.c_str(), -1, &pQuery, NULL);
  // prepare the statement
  if (ret != SQLITE_OK) {
    error = "Failed to prepare select count: " + std::string(sqlite3_errmsg((sqlite3 *)m_db)) +
            "\n SQL-query: " + sql;
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
  if (!copy_database_to_backup(error)) {
    return nullptr;
  }
  sqlite3_stmt *pQuery = nullptr;
  int res = sqlite3_prepare_v2((sqlite3 *)m_db, sql.c_str(), -1, &pQuery, NULL);
  // prepare the statement
  if (res != SQLITE_OK) {
    error = "Failed to prepare select rows: " + std::string(sqlite3_errmsg((sqlite3 *)m_db)) +
            "\n SQL-query: " + sql;
    return nullptr;
  }
  auto ret = std::make_shared<impl_rows_iterator>();
  ret->set_stmt(pQuery);
  return ret;
}

bool database_file::copy_database_to_backup(std::string &error) {
  std::lock_guard<std::mutex> lock(m_mutex);
  // every 1 minutes make backup
  int nCurrentTime = WsjcppCore::getCurrentTimeInSeconds();
  if (nCurrentTime - m_last_backup_time < m_backup_freq_in_seconds) {
    return true;
  }
  m_last_backup_time = nCurrentTime;

  int nMaxBackupsFiles = 9;
  sea5kg::log::info(TAG, "Start backup for " + m_sFileFullpath);
  std::string sFilebackup = m_sBaseFileBackupFullpath + "." + std::to_string(nMaxBackupsFiles);
  if (wsjcpp::file_exists(sFilebackup)) {
    WsjcppCore::removeFile(sFilebackup);
  }
  for (int i = nMaxBackupsFiles - 1; i >= 0; i--) {
    std::string sFilebackupFrom = m_sBaseFileBackupFullpath + "." + std::to_string(i);
    std::string sFilebackupTo = m_sBaseFileBackupFullpath + "." + std::to_string(i + 1);
    if (wsjcpp::file_exists(sFilebackupFrom)) {
      if (std::rename(sFilebackupFrom.c_str(), sFilebackupTo.c_str())) {
        sea5kg::log::critical(TAG, "Could not rename from " + sFilebackupFrom + " to " + sFilebackupTo);
        return false;
      }
    }
  }
  sFilebackup = m_sBaseFileBackupFullpath + "." + std::to_string(0);
  if (!WsjcppCore::copyFile(m_sFileFullpath, sFilebackup)) {
    sea5kg::log::critical(TAG, "Failed copy file to backup for " + m_sFileFullpath);
  }
  sea5kg::log::info(TAG, "Backup done for " + m_sFileFullpath);
  return true;
}

} // namespace sqlite3_wrapper

} // namespace sea5kg
