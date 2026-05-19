/**********************************************************************************
 *          Project
 *  _______ _________ _______  _______  __    ______
 * (  ____ \\__   __/(  ____ \(  __   )/  \  (  __  \
 * | (    \/   ) (   | (    \/| (  )  |\/) ) | (  \  )
 * | |         | |   | (__    | | /   |  | | | |   ) |
 * | |         | |   |  __)   | (/ /) |  | | | |   | |
 * | |         | |   | (      |   / | |  | | | |   ) |
 * | (____/\   | |   | )      |  (__) |__) (_| (__/  )
 * (_______/   )_(   |/       (_______)\____/(______/
 *
 * MIT License
 * Copyright (c) 2018-2025 Evgenii Sopov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 ***********************************************************************************/

#include "ctf01d_database_file.h"
#include <sqlite3.h>
#include <wsjcpp_core.h>
#include <wsjcpp_employees.h>
#include <employ_config.h>

// ---------------------------------------------------------------------
// Ctf01dDatabase

std::map<std::string, Ctf01dDatabaseFile *> *g_pOpenedDatabaseFiles = nullptr;

// static
void Ctf01dDatabase::addOpenedDatabaseFile(const std::string &name, Ctf01dDatabaseFile *db) {
  if (g_pOpenedDatabaseFiles == nullptr) {
    // WsjcppLog::info(std::string(), "Create employees map");
    g_pOpenedDatabaseFiles = new std::map<std::string, Ctf01dDatabaseFile*>();
  }
  if (g_pOpenedDatabaseFiles->find(name) != g_pOpenedDatabaseFiles->end()) {
    WsjcppLog::throw_err("WsjcppEmployees::addService", "Already registered '" + name + "'");
  } else {
    g_pOpenedDatabaseFiles->insert(std::pair<std::string, Ctf01dDatabaseFile*>(name, db));
  }
}

// static
bool Ctf01dDatabase::initDriverSqlite3(int &ret) {
  ret = sqlite3_initialize();
  return SQLITE_OK == ret;
}

// static
void Ctf01dDatabase::shutdownDriverSqlite3() {
  // will be automatilly closed all opened databases
  if (g_pOpenedDatabaseFiles != nullptr) {
    for (const auto& pair : *g_pOpenedDatabaseFiles) {
      pair.second->close();
    }
  }
  sqlite3_shutdown();
}

// ---------------------------------------------------------------------
// Impl_Ctf01dDatabaseSelectRows

class Impl_Ctf01dDatabaseSelectRows : public Ctf01dDatabaseSelectRows {
public:
  Impl_Ctf01dDatabaseSelectRows();
  ~Impl_Ctf01dDatabaseSelectRows();
  void setQuery(sqlite3_stmt* pQuery);
  virtual bool next() override;
  virtual std::string getString(int nColumnNumber) override;
  virtual long getLong(int nColumnNumber) override;

private:
  sqlite3_stmt* m_pQuery;
};


Impl_Ctf01dDatabaseSelectRows::Impl_Ctf01dDatabaseSelectRows() {
  m_pQuery = nullptr;
}

Impl_Ctf01dDatabaseSelectRows::~Impl_Ctf01dDatabaseSelectRows() {
  if (m_pQuery != nullptr) {
    sqlite3_finalize(m_pQuery);
  }
}

void Impl_Ctf01dDatabaseSelectRows::setQuery(sqlite3_stmt* pQuery) {
  m_pQuery = pQuery;
}

bool Impl_Ctf01dDatabaseSelectRows::next() {
  return  sqlite3_step(m_pQuery) == SQLITE_ROW;
}

std::string Impl_Ctf01dDatabaseSelectRows::getString(int nColumnNumber) {
  return std::string((const char *)sqlite3_column_text(m_pQuery, nColumnNumber));
}

long Impl_Ctf01dDatabaseSelectRows::getLong(int nColumnNumber) {
  return sqlite3_column_int64(m_pQuery, nColumnNumber);
}

// ---------------------------------------------------------------------
// Ctf01dDatabaseFile

Ctf01dDatabaseFile::Ctf01dDatabaseFile(const std::string &sFilename, const std::string &sSqlCreateTable) {
  TAG = "Ctf01dDatabaseFile-" + sFilename;
  m_pDatabaseFile = nullptr;
  m_sFilename = sFilename;
  std::string sError;
  m_nLastBackupTime = 0;
  m_sSqlCreateTable = sSqlCreateTable;
  EmployConfig *pConfig = findWsjcppEmploy<EmployConfig>();
  std::string sDatabaseDir = pConfig->getWorkDir() + "/db";
  if (!WsjcppCore::dirExists(sDatabaseDir)) {
    if (!WsjcppCore::makeDir(sDatabaseDir)) {
      WsjcppLog::throw_err(TAG, "Could not create dir " + sDatabaseDir);
    }
    if (!WsjcppCore::setFilePermissions(sDatabaseDir, WsjcppFilePermissions(0x776), sError)) {
      WsjcppLog::throw_err(TAG, sError);
    }
  }
  m_sFileFullpath = sDatabaseDir + "/" + m_sFilename;

  std::string sDatabaseBackupDir = sDatabaseDir + "/backups";
  if (!WsjcppCore::dirExists(sDatabaseBackupDir)) {
    if (!WsjcppCore::makeDir(sDatabaseBackupDir)) {
      WsjcppLog::throw_err(TAG, "Could not create dir " + sDatabaseBackupDir);
    }
    if (!WsjcppCore::setFilePermissions(sDatabaseBackupDir, WsjcppFilePermissions(0x776), sError)) {
      WsjcppLog::throw_err(TAG, sError);
    }
  }
  m_sBaseFileBackupFullpath = sDatabaseBackupDir + "/" + m_sFilename;
};

Ctf01dDatabaseFile::~Ctf01dDatabaseFile() {
  close();
}

bool Ctf01dDatabaseFile::open() {
  // open connection to a DB
  sqlite3 *db = (sqlite3 *)m_pDatabaseFile;
  int nRet = sqlite3_open_v2(
    m_sFileFullpath.c_str(),
    &db,
    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
    NULL
  );
  if (nRet != SQLITE_OK) {
    WsjcppLog::throw_err(TAG, "Failed to open conn: " + std::to_string(nRet));
    return false;
  }
  m_pDatabaseFile = db;

  // Run the SQL (convert the string to a C-String with c_str() )
  char *zErrMsg = 0;
  nRet = sqlite3_exec((sqlite3 *)m_pDatabaseFile, m_sSqlCreateTable.c_str(), 0, 0, &zErrMsg);
  if (nRet != SQLITE_OK) {
    WsjcppLog::err(TAG, "Could not create table: " + m_sSqlCreateTable);
    std::string error_msg = "";
    if (zErrMsg != 0) {
      error_msg = std::string(zErrMsg);
    }
    WsjcppLog::throw_err(TAG, "Problem with create table: " + error_msg);
    return false;
  }
  WsjcppLog::ok(TAG, "Opened database file " + m_sFileFullpath);
  copyDatabaseToBackup();
  Ctf01dDatabase::addOpenedDatabaseFile(m_sFileFullpath, this);
  return true;
}

void Ctf01dDatabaseFile::close() {
  if (m_pDatabaseFile != nullptr) {
    sqlite3_close((sqlite3 *)m_pDatabaseFile);
    m_pDatabaseFile = nullptr;
  }
}

bool Ctf01dDatabaseFile::executeQuery(std::string sSqlInsert) {
  copyDatabaseToBackup();
  char *zErrMsg = 0;
  int nRet = sqlite3_exec((sqlite3 *)m_pDatabaseFile, sSqlInsert.c_str(), 0, 0, &zErrMsg);
  if (nRet != SQLITE_OK) {
    WsjcppLog::throw_err(TAG, "Problem with insert: " + std::string(zErrMsg) + "\n SQL-query: " + sSqlInsert);
    return false;
  }
  return true;
}

int Ctf01dDatabaseFile::selectSumOrCount(std::string sSqlSelectCount) {
  copyDatabaseToBackup();
  sqlite3_stmt* pQuery = nullptr;
  int ret = sqlite3_prepare_v2((sqlite3 *)m_pDatabaseFile, sSqlSelectCount.c_str(), -1, &pQuery, NULL);
  // prepare the statement
  if (ret != SQLITE_OK) {
    WsjcppLog::throw_err(TAG, "Failed to prepare select count: " + std::string(sqlite3_errmsg((sqlite3 *)m_pDatabaseFile)) + "\n SQL-query: " + sSqlSelectCount);
  }
  // step to 1st row of data
  ret = sqlite3_step(pQuery);
  if (ret != SQLITE_ROW) { // see documentation, this can return more values as success
    WsjcppLog::throw_err(TAG, "Failed to step for select count or sum: " + std::string(sqlite3_errmsg((sqlite3 *)m_pDatabaseFile)) + "\n SQL-query: " + sSqlSelectCount);
  }
  int nRet = sqlite3_column_int(pQuery, 0);
  if (pQuery != nullptr) sqlite3_finalize(pQuery);
  return nRet;
}

std::shared_ptr<Ctf01dDatabaseSelectRows> Ctf01dDatabaseFile::selectRows(std::string sqlSelectRows) {
  copyDatabaseToBackup();
  sqlite3_stmt* pQuery = nullptr;
  int nRet = sqlite3_prepare_v2((sqlite3 *)m_pDatabaseFile, sqlSelectRows.c_str(), -1, &pQuery, NULL);
  // prepare the statement
  if (nRet != SQLITE_OK) {
    WsjcppLog::throw_err(TAG, "Failed to prepare select rows: " + std::string(sqlite3_errmsg((sqlite3 *)m_pDatabaseFile)) + "\n SQL-query: " + sqlSelectRows);
    return nullptr;
  }
  auto selectRows = std::make_shared<Impl_Ctf01dDatabaseSelectRows>();
  selectRows->setQuery(pQuery);
  return selectRows;
}

void Ctf01dDatabaseFile::copyDatabaseToBackup() {
  std::lock_guard<std::mutex> lock(m_mutex);
  // every 1 minutes make backup
  int nCurrentTime = WsjcppCore::getCurrentTimeInSeconds();
  if (nCurrentTime - m_nLastBackupTime < 60) {
    return;
  }
  m_nLastBackupTime = nCurrentTime;

  int nMaxBackupsFiles = 9;
  WsjcppLog::info(TAG, "Start backup for " + m_sFileFullpath);
  std::string sFilebackup = m_sBaseFileBackupFullpath + "." + std::to_string(nMaxBackupsFiles);
  if (WsjcppCore::fileExists(sFilebackup)) {
    WsjcppCore::removeFile(sFilebackup);
  }
  for (int i = nMaxBackupsFiles - 1; i >= 0; i--) {
    std::string sFilebackupFrom = m_sBaseFileBackupFullpath + "." + std::to_string(i);
    std::string sFilebackupTo = m_sBaseFileBackupFullpath + "." + std::to_string(i+1);
    if (WsjcppCore::fileExists(sFilebackupFrom)) {
      if (std::rename(sFilebackupFrom.c_str(), sFilebackupTo.c_str())) {
        WsjcppLog::throw_err(TAG, "Could not rename from " + sFilebackupFrom + " to " + sFilebackupTo);
      }
    }
  }
  sFilebackup = m_sBaseFileBackupFullpath + "." + std::to_string(0);
  if (!WsjcppCore::copyFile(m_sFileFullpath, sFilebackup)) {
    WsjcppLog::throw_err(TAG, "Failed copy file to backup for " + m_sFileFullpath);
  }
  WsjcppLog::info(TAG, "Backup done for " + m_sFileFullpath);
}
