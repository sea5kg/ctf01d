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

#include "employ_database.h"
#include <wsjcpp_core.h>
#include "ctf01d/employees/employ_config.h"
#include <cmath>
#include <stdio.h>
#include <string>
#include <map>
#include <mutex>
#include <vector>

REGISTRY_WSJCPP_EMPLOY(EmployDatabase)

EmployDatabase::EmployDatabase()
: WsjcppEmployBase({ EmployDatabase::name() }, { EmployConfig::name() }) {
    TAG = EmployDatabase::name();
    m_pFlagsAttempts = nullptr;
    m_pFlagsDefense = nullptr;
    m_pFlagsCheckFails = nullptr;
    m_pFlagsStolen = nullptr;
    m_pFlagsCheckerPutsResults = nullptr;
}

bool EmployDatabase::init(const std::string &sName, bool bSilent) {
    int driver_init_ret;
    if (!Ctf01dDatabase::initDriverSqlite3(driver_init_ret)) {
        WsjcppLog::throw_err(TAG, "Failed to initialize build-in sqlite3 library: " + std::to_string(driver_init_ret));
        return false;
    }
    WsjcppLog::ok(TAG, "Initialize build-in sqlite3 library");

    m_pFlagsCheckerPutsResults = std::make_shared<Ctf01dDatabaseFile>("flags_checker_put_results.db",
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
    WsjcppLog::info(TAG, "Opening m_pFlagsCheckerPutsResults");
    if (!m_pFlagsCheckerPutsResults->open()) {
        return false;
    }

    m_pFlagsAttempts = std::make_shared<Ctf01dDatabaseFile>("flags_attempts.db",
        "CREATE TABLE IF NOT EXISTS flags_attempts ( "
        "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "  flag VARCHAR(36) NOT NULL, "
        "  teamid VARCHAR(50) NOT NULL, "
        "  request_ip VARCHAR(50) NOT NULL, "
        "  dt INTEGER NOT NULL"
        ");"
    );
    WsjcppLog::info(TAG, "Opening m_pFlagsAttempts");
    if (!m_pFlagsAttempts->open()) {
        return false;
    }

    m_pFlagsDefense = std::make_shared<Ctf01dDatabaseFile>("flags_defense.db",
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
    WsjcppLog::info(TAG, "Opening m_pFlagsDefense");
    if (!m_pFlagsDefense->open()) {
        return false;
    }

    m_pFlagsCheckFails = std::make_shared<Ctf01dDatabaseFile>("flags_check_fails.db",
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
    WsjcppLog::info(TAG, "Opening m_pFlagsCheckFails");
    if (!m_pFlagsCheckFails->open()) {
        return false;
    }

    m_pFlagsStolen = std::make_shared<Ctf01dDatabaseFile>("flags_stolen.db",
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
    WsjcppLog::info(TAG, "Opening m_pFlagsStolen");
    if (!m_pFlagsStolen->open()) {
        return false;
    }

    return true;
}

bool EmployDatabase::deinit(const std::string &sName, bool bSilent) {
    WsjcppLog::info(TAG, "deinit");
    Ctf01dDatabase::shutdownDriverSqlite3();
    return true;
}

void EmployDatabase::insertToFlagsCheckerPutResult(ctf01d::flag flag, std::string sResult) {
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

void EmployDatabase::insertToFlagsDefense(ctf01d::flag flag, int nPoints) {
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
        WsjcppLog::err(TAG, "Error insert insertToFlagsDefense");
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

int EmployDatabase::numberOfDefenseFlagForService(std::string sServiceId) {
    return m_pFlagsDefense->selectSumOrCount(
        "SELECT COUNT(*) as cnt FROM flags_defense WHERE serviceid = '" + sServiceId + "'"
    );
}

void EmployDatabase::insertFlagCheckFail(ctf01d::flag flag, std::string sReason) {
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
        WsjcppLog::err(TAG, "Error insert insertToFlagsDefense");
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

std::pair<std::string, long> EmployDatabase::getFirstBloodFromStolenFlagsForService(std::string sServiceId) {
    std::string sQuery = "SELECT thief_teamid, date_action FROM flags_stolen WHERE serviceid = '" + sServiceId + "' LIMIT 1";
    std::pair<std::string, long> pairRet;
    pairRet.first = "?";
    pairRet.second = 0;
    auto rows = m_pFlagsStolen->selectRows(sQuery);
    if (rows == nullptr) {
        WsjcppLog::err(TAG, "Error select getFirstBloodFromStolenFlagsForService " + sQuery);
        return pairRet;
    }
    if (rows->next()) {
        pairRet.first = rows->getString(0);
        pairRet.second = rows->getLong(1);
    }
    return pairRet;
}

void EmployDatabase::insertToFlagsStolen(ctf01d::flag flag, std::string sTeamId, int nPoints, long nDateAction, int nVictimPlaceInScoreBoard, int nThiefPlaceInScoreboard) {
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
        WsjcppLog::err(TAG, "Error insert insertToFlagsDefense");
    }
}


bool EmployDatabase::isAlreadyStole(ctf01d::flag flag, std::string sTeamId) {
    int nRet = m_pFlagsStolen->selectSumOrCount(
        "SELECT COUNT(*) as cnt FROM flags_stolen "
            " WHERE serviceid = '" + flag.getServiceId() + "' "
            "   AND thief_teamid = '" + sTeamId + "'"
            "   AND flag_id = '" + flag.getId() + "'"
            "   AND flag = '" + flag.getValue() + "'"
    );
    return nRet > 0;
}

bool EmployDatabase::isSomebodyStole(ctf01d::flag flag) {
    int nRet = m_pFlagsStolen->selectSumOrCount(
        "SELECT COUNT(*) as cnt FROM flags_stolen "
            " WHERE serviceid = '" + flag.getServiceId() + "' "
            "   AND teamid = '" + flag.getTeamId() + "'"
            "   AND flag_id = '" + flag.getId() + "'"
            "   AND flag = '" + flag.getValue() + "'"
    );
    return nRet > 0;
}
