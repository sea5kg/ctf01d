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

#include "ctf01d_database_file.h"
#include <sqlite3.h>
#include <wsjcpp_core.h>
#include <wsjcpp_employees.h>
#include "ctf01d/include/ctf01d_config.h"
#include "ctf01d/utils/ctf01d_logger.h"

namespace ctf01d {

std::map<std::string, database_file *> *g_opened_database_files = nullptr;

// static
void global_databases::add_opened_database_file(const std::string &name, database_file *db) {
  if (g_opened_database_files == nullptr) {
    // ctf01d::log::info(std::string(), "Create employees map");
    g_opened_database_files = new std::map<std::string, database_file*>();
  }
  if (g_opened_database_files->find(name) != g_opened_database_files->end()) {
    ctf01d::log::throw_err("WsjcppEmployees::addService", "Already registered '" + name + "'");
  } else {
    g_opened_database_files->insert(std::pair<std::string, database_file*>(name, db));
  }
}

// static
bool global_databases::init_driver_sqlite3(int &ret) {
  ret = sqlite3_initialize();
  return SQLITE_OK == ret;
}

// static
void global_databases::shutdown_driver_sqlite3() {
  // will be automatically closed all opened databases
  if (g_opened_database_files != nullptr) {
    for (const auto& pair : *g_opened_database_files) {
      pair.second->close();
    }
  }
  sqlite3_shutdown();
}

class impl_database_select_rows : public database_select_rows {
public:
  impl_database_select_rows();
  ~impl_database_select_rows();
  void setQuery(sqlite3_stmt* pQuery);
  virtual bool next() override;
  virtual std::string getString(int nColumnNumber) override;
  virtual long getLong(int nColumnNumber) override;

private:
  sqlite3_stmt* m_pQuery;
};


impl_database_select_rows::impl_database_select_rows() {
  m_pQuery = nullptr;
}

impl_database_select_rows::~impl_database_select_rows() {
  if (m_pQuery != nullptr) {
    sqlite3_finalize(m_pQuery);
  }
}

void impl_database_select_rows::setQuery(sqlite3_stmt* pQuery) {
  m_pQuery = pQuery;
}

bool impl_database_select_rows::next() {
  return  sqlite3_step(m_pQuery) == SQLITE_ROW;
}

std::string impl_database_select_rows::getString(int nColumnNumber) {
  return std::string((const char *)sqlite3_column_text(m_pQuery, nColumnNumber));
}

long impl_database_select_rows::getLong(int nColumnNumber) {
  return sqlite3_column_int64(m_pQuery, nColumnNumber);
}

// ---------------------------------------------------------------------
// database_file

database_file::database_file(const std::string &sFilename, const std::string &sSqlCreateTable) {
  TAG = "database_file-" + sFilename;
  m_database_file_db = nullptr;
  m_sFilename = sFilename;
  std::string sError;
  m_nLastBackupTime = 0;
  m_sSqlCreateTable = sSqlCreateTable;
  auto config = findWsjcppEmploy<ctf01d::config>();
  std::string sDatabaseDir = config->get_work_dir() + "/db";
  if (!WsjcppCore::dirExists(sDatabaseDir)) {
    if (!WsjcppCore::makeDir(sDatabaseDir)) {
      ctf01d::log::throw_err(TAG, "Could not create dir " + sDatabaseDir);
    }
    if (!WsjcppCore::setFilePermissions(sDatabaseDir, WsjcppFilePermissions(0x776), sError)) {
      ctf01d::log::throw_err(TAG, sError);
    }
  }
  m_sFileFullpath = sDatabaseDir + "/" + m_sFilename;

  std::string sDatabaseBackupDir = sDatabaseDir + "/backups";
  if (!WsjcppCore::dirExists(sDatabaseBackupDir)) {
    if (!WsjcppCore::makeDir(sDatabaseBackupDir)) {
      ctf01d::log::throw_err(TAG, "Could not create dir " + sDatabaseBackupDir);
    }
    if (!WsjcppCore::setFilePermissions(sDatabaseBackupDir, WsjcppFilePermissions(0x776), sError)) {
      ctf01d::log::throw_err(TAG, sError);
    }
  }
  m_sBaseFileBackupFullpath = sDatabaseBackupDir + "/" + m_sFilename;
};

database_file::~database_file() {
  close();
}

bool database_file::open() {
  // open connection to a DB
  sqlite3 *db = (sqlite3 *)m_database_file_db;
  int nRet = sqlite3_open_v2(
    m_sFileFullpath.c_str(),
    &db,
    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
    NULL
  );
  if (nRet != SQLITE_OK) {
    ctf01d::log::throw_err(TAG, "Failed to open conn: " + std::to_string(nRet));
    return false;
  }
  m_database_file_db = db;

  // Run the SQL (convert the string to a C-String with c_str() )
  char *zErrMsg = 0;
  nRet = sqlite3_exec((sqlite3 *)m_database_file_db, m_sSqlCreateTable.c_str(), 0, 0, &zErrMsg);
  if (nRet != SQLITE_OK) {
    ctf01d::log::err(TAG, "Could not create table: " + m_sSqlCreateTable);
    std::string error_msg = "";
    if (zErrMsg != 0) {
      error_msg = std::string(zErrMsg);
    }
    ctf01d::log::throw_err(TAG, "Problem with create table: " + error_msg);
    return false;
  }
  ctf01d::log::ok(TAG, "Opened database file " + m_sFileFullpath);
  copy_database_to_backup();
  ctf01d::global_databases::add_opened_database_file(m_sFileFullpath, this);
  return true;
}

void database_file::close() {
  if (m_database_file_db != nullptr) {
    sqlite3_close((sqlite3 *)m_database_file_db);
    m_database_file_db = nullptr;
  }
}

bool database_file::executeQuery(std::string sql_query) {
  copy_database_to_backup();
  char *errMsg = 0;
  int nRet = sqlite3_exec((sqlite3 *)m_database_file_db, sql_query.c_str(), 0, 0, &errMsg);
  if (nRet != SQLITE_OK) {
    ctf01d::log::throw_err(TAG, "Problem with insert: " + std::string(errMsg) + "\n SQL-query: " + sql_query);
    sqlite3_free(errMsg);
    return false;
  }
  return true;
}

int database_file::selectSumOrCount(std::string sSqlSelectCount) {
  copy_database_to_backup();
  sqlite3_stmt* pQuery = nullptr;
  int ret = sqlite3_prepare_v2((sqlite3 *)m_database_file_db, sSqlSelectCount.c_str(), -1, &pQuery, NULL);
  // prepare the statement
  if (ret != SQLITE_OK) {
    ctf01d::log::throw_err(TAG, "Failed to prepare select count: " + std::string(sqlite3_errmsg((sqlite3 *)m_database_file_db)) + "\n SQL-query: " + sSqlSelectCount);
  }
  // step to 1st row of data
  ret = sqlite3_step(pQuery);
  if (ret != SQLITE_ROW) { // see documentation, this can return more values as success
    ctf01d::log::throw_err(TAG, "Failed to step for select count or sum: " + std::string(sqlite3_errmsg((sqlite3 *)m_database_file_db)) + "\n SQL-query: " + sSqlSelectCount);
  }
  int nRet = sqlite3_column_int(pQuery, 0);
  if (pQuery != nullptr) sqlite3_finalize(pQuery);
  return nRet;
}

std::shared_ptr<database_select_rows> database_file::selectRows(std::string sqlSelectRows) {
  copy_database_to_backup();
  sqlite3_stmt* pQuery = nullptr;
  int nRet = sqlite3_prepare_v2((sqlite3 *)m_database_file_db, sqlSelectRows.c_str(), -1, &pQuery, NULL);
  // prepare the statement
  if (nRet != SQLITE_OK) {
    ctf01d::log::throw_err(TAG, "Failed to prepare select rows: " + std::string(sqlite3_errmsg((sqlite3 *)m_database_file_db)) + "\n SQL-query: " + sqlSelectRows);
    return nullptr;
  }
  auto selectRows = std::make_shared<impl_database_select_rows>();
  selectRows->setQuery(pQuery);
  return selectRows;
}

void database_file::copy_database_to_backup() {
  std::lock_guard<std::mutex> lock(m_mutex);
  // every 1 minutes make backup
  int nCurrentTime = WsjcppCore::getCurrentTimeInSeconds();
  if (nCurrentTime - m_nLastBackupTime < 60) {
    return;
  }
  m_nLastBackupTime = nCurrentTime;

  int nMaxBackupsFiles = 9;
  ctf01d::log::info(TAG, "Start backup for " + m_sFileFullpath);
  std::string sFilebackup = m_sBaseFileBackupFullpath + "." + std::to_string(nMaxBackupsFiles);
  if (WsjcppCore::fileExists(sFilebackup)) {
    WsjcppCore::removeFile(sFilebackup);
  }
  for (int i = nMaxBackupsFiles - 1; i >= 0; i--) {
    std::string sFilebackupFrom = m_sBaseFileBackupFullpath + "." + std::to_string(i);
    std::string sFilebackupTo = m_sBaseFileBackupFullpath + "." + std::to_string(i+1);
    if (WsjcppCore::fileExists(sFilebackupFrom)) {
      if (std::rename(sFilebackupFrom.c_str(), sFilebackupTo.c_str())) {
        ctf01d::log::throw_err(TAG, "Could not rename from " + sFilebackupFrom + " to " + sFilebackupTo);
      }
    }
  }
  sFilebackup = m_sBaseFileBackupFullpath + "." + std::to_string(0);
  if (!WsjcppCore::copyFile(m_sFileFullpath, sFilebackup)) {
    ctf01d::log::throw_err(TAG, "Failed copy file to backup for " + m_sFileFullpath);
  }
  ctf01d::log::info(TAG, "Backup done for " + m_sFileFullpath);
}

} // namespace ctf01d
