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

#include "employ_database.h"
#include <wsjcpp_core.h>
#include <employ_config.h>
#include <cmath>
#include <stdio.h>
#include <string>
#include <map>
#include <mutex>
#include <vector>

// ---------------------------------------------------------------------
// Ctf01dDatabaseSelectRows

Ctf01dDatabaseSelectRows::Ctf01dDatabaseSelectRows() {
    m_pQuery = nullptr;
}

Ctf01dDatabaseSelectRows::~Ctf01dDatabaseSelectRows() {
    if (m_pQuery != nullptr) {
        sqlite3_finalize(m_pQuery);
    }
}

void Ctf01dDatabaseSelectRows::setQuery(sqlite3_stmt* pQuery) {
    m_pQuery = pQuery;
}

bool Ctf01dDatabaseSelectRows::next() {
    return  sqlite3_step(m_pQuery) == SQLITE_ROW;
}

std::string Ctf01dDatabaseSelectRows::getString(int nColumnNumber) {
    return std::string((const char *)sqlite3_column_text(m_pQuery, nColumnNumber));
}

long Ctf01dDatabaseSelectRows::getLong(int nColumnNumber) {
    return sqlite3_column_int64(m_pQuery, nColumnNumber);
}

// ---------------------------------------------------------------------
// Ctf01dDatabaseFile

Ctf01dDatabaseFile::Ctf01dDatabaseFile(const std::string &sFilename, const std::string &sSqlCreateTable) {
    TAG = "Ctf01dDatabaseFile-" + sFilename;
    m_pDatabaseFile = nullptr;
    m_sFilename = sFilename;
    m_nLastBackupTime = 0;
    m_sSqlCreateTable = sSqlCreateTable;
    EmployConfig *pConfig = findWsjcppEmploy<EmployConfig>();
    std::string sDatabaseDir = pConfig->getWorkDir() + "/db";
    if (!WsjcppCore::dirExists(sDatabaseDir)) {
        !WsjcppCore::makeDir(sDatabaseDir);
    }
    m_sFileFullpath = sDatabaseDir + "/" + m_sFilename;

    std::string sDatabaseBackupDir = sDatabaseDir + "/backups";
    if (!WsjcppCore::dirExists(sDatabaseBackupDir)) {
        !WsjcppCore::makeDir(sDatabaseBackupDir);
    }
    m_sBaseFileBackupFullpath = sDatabaseBackupDir + "/" + m_sFilename;
};

Ctf01dDatabaseFile::~Ctf01dDatabaseFile() {
    if (m_pDatabaseFile != nullptr) {
        sqlite3_close(m_pDatabaseFile);
    }
}

bool Ctf01dDatabaseFile::open() {
    // open connection to a DB
    int nRet = sqlite3_open_v2(
        m_sFileFullpath.c_str(),
        &m_pDatabaseFile,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
        NULL
    );
    if (nRet != SQLITE_OK) {
        WsjcppLog::throw_err(TAG, "Failed to open conn: " + std::to_string(nRet));
        return false;
    }
    // Run the SQL (convert the string to a C-String with c_str() )
    char *zErrMsg = 0;
    nRet = sqlite3_exec(m_pDatabaseFile, m_sSqlCreateTable.c_str(), 0, 0, &zErrMsg);
    if (nRet != SQLITE_OK) {
        WsjcppLog::throw_err(TAG, "Problem with create table: " + std::string(zErrMsg));
        return false;
    }
    WsjcppLog::ok(TAG, "Opened database file " + m_sFileFullpath);
    copyDatabaseToBackup();
    return true;
}

bool Ctf01dDatabaseFile::executeQuery(std::string sSqlInsert) {
    copyDatabaseToBackup();
    char *zErrMsg = 0;
    int nRet = sqlite3_exec(m_pDatabaseFile, sSqlInsert.c_str(), 0, 0, &zErrMsg);
    if (nRet != SQLITE_OK) {
        WsjcppLog::throw_err(TAG, "Problem with insert: " + std::string(zErrMsg) + "\n SQL-query: " + sSqlInsert);
        return false;
    }
    return true;
}

int Ctf01dDatabaseFile::selectSumOrCount(std::string sSqlSelectCount) {
    copyDatabaseToBackup();
    sqlite3_stmt* pQuery = nullptr;
    int ret = sqlite3_prepare_v2(m_pDatabaseFile, sSqlSelectCount.c_str(), -1, &pQuery, NULL);
    // prepare the statement
    if (ret != SQLITE_OK) {
        WsjcppLog::throw_err(TAG, "Failed to prepare select count: " + std::string(sqlite3_errmsg(m_pDatabaseFile)) + "\n SQL-query: " + sSqlSelectCount);
    }
    // step to 1st row of data
    ret = sqlite3_step(pQuery);
    if (ret != SQLITE_ROW) { // see documentation, this can return more values as success
        WsjcppLog::throw_err(TAG, "Failed to step for select count or sum: " + std::string(sqlite3_errmsg(m_pDatabaseFile)) + "\n SQL-query: " + sSqlSelectCount);
    }
    int nRet = sqlite3_column_int(pQuery, 0);
    if (pQuery != nullptr) sqlite3_finalize(pQuery);
    return nRet;
}

bool Ctf01dDatabaseFile::selectRows(std::string sSqlSelectRows, Ctf01dDatabaseSelectRows &selectRows) {
    copyDatabaseToBackup();
    sqlite3_stmt* pQuery = nullptr;
    int nRet = sqlite3_prepare_v2(m_pDatabaseFile, sSqlSelectRows.c_str(), -1, &pQuery, NULL);
    // prepare the statement
    if (nRet != SQLITE_OK) {
        WsjcppLog::throw_err(TAG, "Failed to prepare select rows: " + std::string(sqlite3_errmsg(m_pDatabaseFile)) + "\n SQL-query: " + sSqlSelectRows);
        return false;
    }
    selectRows.setQuery(pQuery);
    return true;
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

// ---------------------------------------------------------------------
// EmployDatabase

REGISTRY_WJSCPP_SERVICE_LOCATOR(EmployDatabase)

EmployDatabase::EmployDatabase()
: WsjcppEmployBase(EmployDatabase::name(), {"EmployConfig"}) {
    TAG = EmployDatabase::name();
    m_pFlagsAttempts = nullptr;
    m_pFlagsDefense = nullptr;
    m_pFlagsCheckFails = nullptr;
    m_pFlagsStolen = nullptr;
    m_pFlagsLive = nullptr;
    m_pFlagsCheckerPutsResults = nullptr;
}

bool EmployDatabase::init() {
    int nRet = 0;
    if (SQLITE_OK != (nRet = sqlite3_initialize())) {
        WsjcppLog::throw_err(TAG, "Failed to initialize build-in sqlite3 library: " + std::to_string(nRet));
        return false;
    }
    WsjcppLog::ok(TAG, "Initialize build-in sqlite3 library");

    m_pFlagsCheckerPutsResults = new Ctf01dDatabaseFile("flags_checker_put_results.db",
        "CREATE TABLE IF NOT EXISTS flags_checker_put_results ( "
        "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "  serviceid VARCHAR(50) NOT NULL, "
        "  flag_id VARCHAR(50) NOT NULL, "
        "  flag VARCHAR(36) NOT NULL, "
        "  teamid VARCHAR(50) NOT NULL, "
        "  date_start INTEGER NOT NULL,"
        "  date_end INTEGER NOT NULL,"
        "  result VARCHAR(50) NOT NULL"
        ");"
    );
    if (!m_pFlagsCheckerPutsResults->open()) {
        return false;
    }

    m_pFlagsAttempts = new Ctf01dDatabaseFile("flags_attempts.db",
        "CREATE TABLE IF NOT EXISTS flags_attempts ( "
        "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "  flag VARCHAR(36) NOT NULL, "
        "  teamid VARCHAR(50) NOT NULL, "
        "  request_ip VARCHAR(50) NOT NULL, "
        "  dt INTEGER NOT NULL"
        ");"
    );
    if (!m_pFlagsAttempts->open()) {
        return false;
    }

    m_pFlagsDefense = new Ctf01dDatabaseFile("flags_defense.db",
        "CREATE TABLE IF NOT EXISTS flags_defense ( "
        "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "  serviceid VARCHAR(50) NOT NULL, "
        "  teamid VARCHAR(50) NOT NULL, "
        "  flag_id VARCHAR(50) NOT NULL, "
        "  flag VARCHAR(36) NOT NULL, "
        "  date_start INTEGER NOT NULL, "
        "  date_end INTEGER NOT NULL, "
        "  flag_cost INTEGER NOT NULL"
        ");"
    );
    if (!m_pFlagsDefense->open()) {
        return false;
    }

    m_pFlagsCheckFails = new Ctf01dDatabaseFile("flags_check_fails.db",
        "CREATE TABLE IF NOT EXISTS flags_check_fails ( "
        "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "  serviceid VARCHAR(50) NOT NULL, "
        "  flag_id VARCHAR(50) NOT NULL, "
        "  flag VARCHAR(36) NOT NULL, "
        "  teamid VARCHAR(50) NOT NULL, "
        "  date_start INTEGER NOT NULL, "
        "  date_end INTEGER NOT NULL, "
        "  reason VARCHAR(50) NOT NULL "
        ");"
    );
    if (!m_pFlagsCheckFails->open()) {
        return false;
    }

    m_pFlagsStolen = new Ctf01dDatabaseFile("flags_stolen.db",
        "CREATE TABLE IF NOT EXISTS flags_stolen ( "
        "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "  serviceid VARCHAR(50) NOT NULL, "
        "  teamid VARCHAR(50) NOT NULL, "
        "  thief_teamid VARCHAR(50) NOT NULL, "
        "  flag_id VARCHAR(50) NOT NULL, "
        "  flag VARCHAR(36) NOT NULL, "
        "  date_start INTEGER NOT NULL, "
        "  date_end INTEGER NOT NULL, "
        "  date_action INTEGER NOT NULL, "
        "  flag_cost INTEGER NOT NULL "
        ");"
    );
        // TODO
    // "  INDEX(`serviceid`), "
    // "  INDEX(`serviceid`, `thief_teamid`), "
    // "  UNIQUE KEY(`serviceid`, `thief_teamid`, `flag_id`, `flag`)"
    if (!m_pFlagsStolen->open()) {
        return false;
    }

    m_pFlagsLive = new Ctf01dDatabaseFile("flags_live.db",
        "CREATE TABLE IF NOT EXISTS flags_live ( "
        "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "  serviceid VARCHAR(50) NOT NULL, "
        "  flag_id VARCHAR(50) NOT NULL, "
        "  flag VARCHAR(36) NOT NULL, "
        "  teamid VARCHAR(50) NOT NULL, "
        "  date_start INTEGER NOT NULL, "
        "  date_end INTEGER NOT NULL "
        ");"
    );
    if (!m_pFlagsLive->open()) {
        return false;
    }
    return true;
}

bool EmployDatabase::deinit() {
    WsjcppLog::info(TAG, "deinit");
    delete m_pFlagsAttempts;
    delete m_pFlagsDefense;
    delete m_pFlagsCheckFails;
    delete m_pFlagsStolen;
    delete m_pFlagsLive;
    delete m_pFlagsCheckerPutsResults;
    sqlite3_shutdown();
    return true;
}

void EmployDatabase::insertToFlagsCheckerPutResult(Ctf01dFlag flag, std::string sResult) {
    std::string sQuery = "INSERT INTO flags_checker_put_results(serviceid, flag_id, flag, teamid, "
        "   date_start, date_end, result) VALUES("
        "'" + flag.getServiceId() + "', "
        + "'" + flag.getId() + "', "
        + "'" + flag.getValue() + "', "
        + "'" + flag.getTeamId() + "', "
        + std::to_string(flag.getTimeStartInMs()) + ", "
        + std::to_string(flag.getTimeEndInMs()) + ", "
        + "'" + sResult + "'"
        + ");";
    if (!m_pFlagsCheckerPutsResults->executeQuery(sQuery)) {
        WsjcppLog::err(TAG, "Error insert " + sQuery);
    }
}

int EmployDatabase::numberOfFlagFlagsCheckerPutAllResults(std::string sTeamId, std::string sServiceId) {
    return m_pFlagsCheckerPutsResults->selectSumOrCount(
        "SELECT COUNT(*) as defence FROM flags_checker_put_results "
        "WHERE serviceid = '" + sServiceId + "' "
        "   AND teamid = '" + sTeamId + "' "
        ";"
    );
}

int EmployDatabase::numberOfFlagFlagsCheckerPutSuccessResult(std::string sTeamId, std::string sServiceId) {
    return m_pFlagsCheckerPutsResults->selectSumOrCount(
        "SELECT COUNT(*) as defence FROM flags_checker_put_results "
        "WHERE serviceid = '" + sServiceId + "' "
        "   AND teamid = '" + sTeamId + "' "
        "   AND result = 'up' "
        ";"
    );
}

void EmployDatabase::insertFlagAttempt(std::string sTeamId, std::string sFlag, std::string sRequestIP) {
    std::string sQuery = "INSERT INTO flags_attempts(flag, teamid, request_ip, dt) "
        " VALUES('" + sFlag + "', '" + sTeamId + "', '" + sRequestIP + "', " + std::to_string(WsjcppCore::getCurrentTimeInMilliseconds()) + ");";

    if (!m_pFlagsAttempts->executeQuery(sQuery)) {
        WsjcppLog::err(TAG, "Error insert");
    }
}

int EmployDatabase::numberOfFlagAttempts(std::string sTeamId) {
    return m_pFlagsAttempts->selectSumOrCount(
        "SELECT COUNT(*) FROM flags_attempts WHERE teamid = '" + sTeamId + "';"
    );
}

void EmployDatabase::insertToFlagsDefence(Ctf01dFlag flag, int nPoints) {
    std::string sQuery = "INSERT INTO flags_defense(serviceid, teamid, flag_id, flag, "
        "   date_start, date_end, flag_cost) VALUES("
        "'" + flag.getServiceId() + "', "
        + "'" + flag.getTeamId() + "', "
        + "'" + flag.getId() + "', "
        + "'" + flag.getValue() + "', "
        + std::to_string(flag.getTimeStartInMs()) + ", "
        + std::to_string(flag.getTimeEndInMs()) + ", "
        + std::to_string(nPoints) + " "
        + ");";

    if (!m_pFlagsDefense->executeQuery(sQuery)) {
        WsjcppLog::err(TAG, "Error insert insertToFlagsDefence");
    }
}

int EmployDatabase::numberOfFlagsDefense(std::string sTeamId, std::string sServiceId) {
    return m_pFlagsDefense->selectSumOrCount(
        "SELECT COUNT(*) as defence FROM flags_defense "
        "WHERE serviceid = '" + sServiceId + "' "
        "   AND teamid = '" + sTeamId + "' "
        ";"
    );
}

int EmployDatabase::sumPointsOfFlagsDefense(std::string sTeamId, std::string sServiceId) {
    return m_pFlagsDefense->selectSumOrCount(
        "SELECT SUM(flag_cost) as points FROM flags_defense "
        "WHERE serviceid = '" + sServiceId + "' "
        "   AND teamid = '" + sTeamId + "' "
        ";"
    );
}

int EmployDatabase::numberOfDefenceFlagForService(std::string sServiceId) {
    return m_pFlagsDefense->selectSumOrCount(
        "SELECT COUNT(*) as cnt FROM flags_defense WHERE serviceid = '" + sServiceId + "'"
    );
}

void EmployDatabase::insertFlagCheckFail(Ctf01dFlag flag, std::string sReason) {
    std::string sQuery = "INSERT INTO flags_check_fails(serviceid, flag_id, flag, teamid, "
        "   date_start, date_end, reason) VALUES("
        "'" + flag.getServiceId() + "', "
        + "'" + flag.getId() + "', "
        + "'" + flag.getValue() + "', "
        + "'" + flag.getTeamId() + "', "
        + std::to_string(flag.getTimeStartInMs()) + ", "
        + std::to_string(flag.getTimeEndInMs()) + ", "
        + "'" + sReason + "'"
        + ");";

    if (!m_pFlagsCheckFails->executeQuery(sQuery)) {
        WsjcppLog::err(TAG, "Error insert insertToFlagsDefence");
    }
}


int EmployDatabase::numberOfFlagsStollen(std::string sTeamId, std::string sServiceId) {
    return m_pFlagsStolen->selectSumOrCount(
        "SELECT COUNT(*) as cnt FROM flags_stolen "
        "   WHERE serviceid = '" + sServiceId + "' "
        "   AND thief_teamid = '" + sTeamId + "' "
        ";"
    );
}

int EmployDatabase::sumPointsOfFlagsStollen(std::string sTeamId, std::string sServiceId) {
    return m_pFlagsStolen->selectSumOrCount(
        "SELECT SUM(flag_cost) as points FROM flags_stolen "
        "WHERE serviceid = '" + sServiceId + "' "
        "   AND thief_teamid = '" + sTeamId + "' "
        ";"
    );
}

int EmployDatabase::numberOfStolenFlagsForService(std::string sServiceId) {
    return m_pFlagsStolen->selectSumOrCount(
        "SELECT COUNT(*) as cnt FROM flags_stolen WHERE serviceid = '" + sServiceId + "'"
    );
}

std::pair<std::string, long> EmployDatabase::getFirstbloodFromStolenFlagsForService(std::string sServiceId) {
    std::string sQuery = "SELECT thief_teamid, date_action FROM flags_stolen WHERE serviceid = '" + sServiceId + "' LIMIT 1";
    std::pair<std::string, long> pairRet;
    pairRet.first = "?";
    pairRet.second = 0;
    Ctf01dDatabaseSelectRows selectRows;
    if (!m_pFlagsStolen->selectRows(sQuery, selectRows)) {
        WsjcppLog::err(TAG, "Error select getFirstbloodFromStolenFlagsForService " + sQuery);
    }
    if (selectRows.next()) {
        pairRet.first = selectRows.getString(0);
        pairRet.second = selectRows.getLong(1);
    }
    return pairRet;
}

void EmployDatabase::insertToFlagsStolen(Ctf01dFlag flag, std::string sTeamId, int nPoints, long nDateAction, int nVictimPlaceInScoreBoard, int nThiefPlaceInScoreboard) {
    // TODO
    // nVictimPlaceInScoreBoard
    // nThiefPlaceInScoreboard
    std::string sQuery = "INSERT INTO flags_stolen(serviceid, teamid, thief_teamid, flag_id, flag,"
        "   date_start, date_end, date_action, flag_cost) VALUES("
        "'" + flag.getServiceId() + "', "
        + "'" + flag.getTeamId() + "', "
        + "'" + sTeamId + "', "
        + "'" + flag.getId() + "', "
        + "'" + flag.getValue() + "', "
        + std::to_string(flag.getTimeStartInMs()) + ", "
        + std::to_string(flag.getTimeEndInMs()) + ", "
        + std::to_string(nDateAction) + ", "
        + std::to_string(nPoints) + " "
        + ");";

    if (!m_pFlagsStolen->executeQuery(sQuery)) {
        WsjcppLog::err(TAG, "Error insert insertToFlagsDefence");
    }
}


bool EmployDatabase::isAlreadyStole(Ctf01dFlag flag, std::string sTeamId) {
    int nRet = m_pFlagsStolen->selectSumOrCount(
        "SELECT COUNT(*) as cnt FROM flags_stolen "
            " WHERE serviceid = '" + flag.getServiceId() + "' "
            "   AND thief_teamid = '" + sTeamId + "'"
            "   AND flag_id = '" + flag.getId() + "'"
            "   AND flag = '" + flag.getValue() + "'"
    );
    return nRet > 0;
}

bool EmployDatabase::isSomebodyStole(Ctf01dFlag flag) {
    int nRet = m_pFlagsStolen->selectSumOrCount(
        "SELECT COUNT(*) as cnt FROM flags_stolen "
            " WHERE serviceid = '" + flag.getServiceId() + "' "
            "   AND teamid = '" + flag.getTeamId() + "'"
            "   AND flag_id = '" + flag.getId() + "'"
            "   AND flag = '" + flag.getValue() + "'"
    );
    return nRet > 0;
}

void EmployDatabase::insertToFlagLive(Ctf01dFlag flag) {
    std::string sQuery = "INSERT INTO flags_live(serviceid, flag_id, flag, teamid, "
        "   date_start, date_end) VALUES("
        "'" + flag.getServiceId() + "', "
        + "'" + flag.getId() + "', "
        + "'" + flag.getValue() + "', "
        + "'" + flag.getTeamId() + "', "
        + std::to_string(flag.getTimeStartInMs()) + ", "
        + std::to_string(flag.getTimeEndInMs())
        + ");";
    if (!m_pFlagsLive->executeQuery(sQuery)) {
        WsjcppLog::err(TAG, "Error insert insertToFlagLive");
    }
}

void EmployDatabase::deleteFlagLive(Ctf01dFlag flag) {
    std::string sQuery = "DELETE FROM flags_live WHERE flag = '" + flag.getValue() + "';";
    if (!m_pFlagsLive->executeQuery(sQuery)) {
        WsjcppLog::err(TAG, "Error delete deleteFlagLive");
    }
}

std::vector<Ctf01dFlag> EmployDatabase::listOfLiveFlags() {
    // long nCurrentTime = WsjcppCore::getCurrentTimeInMilliseconds();
    EmployConfig *pConfig = findWsjcppEmploy<EmployConfig>();

    std::string sQuery =
        "SELECT flag_id, serviceid, teamid, flag, date_start, date_end "
        "FROM flags_live "
        "WHERE "
        "   date_start > " + std::to_string(long(pConfig->gameStartUTCInSec())*1000) + " "
        "   AND date_end < " + std::to_string(long(pConfig->gameEndUTCInSec())*1000) + " "
        ";";

    std::vector<Ctf01dFlag> vResult;
    Ctf01dDatabaseSelectRows selectRows;
    if (!m_pFlagsLive->selectRows(sQuery, selectRows)) {
        WsjcppLog::err(TAG, "Error select listOfLiveFlags " + sQuery);
    }
    int nCounter = 0;
    while (selectRows.next()) {
        nCounter++;
        Ctf01dFlag flag;
        std::string sFlagId = selectRows.getString(0);
        flag.setId(sFlagId);
        flag.setServiceId(selectRows.getString(1));
        flag.setTeamId(selectRows.getString(2));
        std::string sFlagValue = selectRows.getString(3);
        flag.setValue(sFlagValue);
        flag.setTimeStartInMs(selectRows.getLong(4));
        flag.setTimeEndInMs(selectRows.getLong(5));
        WsjcppLog::info(TAG, "Loaded flag from previous session flags_live: id = " + sFlagId + ", value = " + sFlagValue);
        vResult.push_back(flag);
    }
    WsjcppLog::info(TAG, "Found rows listOfLiveFlags " + std::to_string(nCounter));
    return vResult;
}
