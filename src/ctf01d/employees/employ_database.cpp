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

#include <wsjcpp_employees.h>
#include <wsjcpp_core.h>
#include "ctf01d/include/ctf01d_database.h"
#include "ctf01d/objects/ctf01d_database_file.h"
#include "ctf01d/objects/ctf01d_flag.h"
#include "ctf01d/include/ctf01d_config.h"
#include "ctf01d/include/ctf01d_globals.h"
#include <sea5kg_logger.h>
#include <cmath>
#include <stdio.h>
#include <string>
#include <map>
#include <mutex>
#include <vector>

class employ_database : public WsjcppEmployBase, public ctf01d::database {
public:
  employ_database();
  virtual bool init(const std::string &name, bool silent) override;
  virtual bool deinit(const std::string &name, bool silent) override;

  // ctf01d::database
  virtual void insert_to_flags_checker_put_result(ctf01d::flag flag, std::string sResult) override;
  virtual int number_of_flags_checker_put_all_results(std::string team_id, std::string service_id) override;
  virtual int number_of_flags_checker_put_success_result(std::string team_id, std::string service_id) override;
  virtual void insertToFlagsDefense(ctf01d::flag flag, int nPoints) override;
  virtual int number_of_flags_defense(std::string team_id, std::string service_id) override;
  virtual int sum_points_of_flags_defense(std::string team_id, std::string service_id) override;
  virtual int number_of_defense_flag_for_service(std::string service_id) override;
  virtual void insert_flag_check_fail(ctf01d::flag flag, std::string sReason) override;
  virtual int number_of_flags_stollen(std::string team_id, std::string service_id) override;
  virtual int number_of_flags_stollen_by_victim(std::string team_id, std::string service_id) override;
  virtual int sum_points_of_flags_stolen(std::string team_id, std::string service_id) override;
  virtual int number_of_stolen_flags_for_service(std::string service_id) override;
  virtual std::pair<std::string, long> get_first_blood_from_stolen_flags_for_service(std::string service_id) override;
  virtual void insert_to_flags_stolen(ctf01d::flag flag, std::string team_id, int nPoints, long date_action, int victim_place_in_scoreboard, int thief_place_in_scoreboard) override;
  virtual bool is_already_stole(ctf01d::flag flag, std::string team_id) override;
  virtual bool is_somebody_stole(ctf01d::flag flag) override;

private:
  std::string TAG;
  std::shared_ptr<ctf01d::database_file> m_flags_defense_db;
  std::shared_ptr<ctf01d::database_file> m_flags_check_fails;
  std::shared_ptr<ctf01d::database_file> m_flags_stolen;
  std::shared_ptr<ctf01d::database_file> m_flags_checker_puts_results;
};

REGISTRY_WSJCPP_EMPLOY(employ_database)

employ_database::employ_database()
: WsjcppEmployBase({ ctf01d::database::name() }, { ctf01d::config::name() }) {
  TAG = ctf01d::database::name();
  m_flags_defense_db = nullptr;
  m_flags_check_fails = nullptr;
  m_flags_stolen = nullptr;
  m_flags_checker_puts_results = nullptr;
}

bool employ_database::init(const std::string &sName, bool bSilent) {
  int driver_init_ret;
  if (!ctf01d::global_databases::init_driver_sqlite3(driver_init_ret)) {
    sea5kg::log::critical(TAG, "Failed to initialize build-in sqlite3 library: " + std::to_string(driver_init_ret));
    return false;
  }
  sea5kg::log::success(TAG, "Initialize build-in sqlite3 library");

  m_flags_checker_puts_results = std::make_shared<ctf01d::database_file>("database_flags_checker_put_results",
    "CREATE TABLE IF NOT EXISTS flags_checker_put_results ( "
    "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "  serviceid VARCHAR(50) NOT NULL, "
    "  flag_id VARCHAR(50) NOT NULL, "
    "  flag VARCHAR(36) NOT NULL, "
    "  team_id VARCHAR(50) NOT NULL, "
    "  date_start INTEGER NOT NULL,"
    "  date_end INTEGER NOT NULL,"
    "  result VARCHAR(50) NOT NULL"
    ");",
    "flags_checker_put_results.db",
    findWsjcppEmploy<ctf01d::config>()->db_dir(),
    ctf01d::DEFAULT_DATABASE_BACKUP_FREQUENCY_IN_SECONDS
  );
  sea5kg::log::info(TAG, "Opening m_flags_checker_puts_results");
  if (!m_flags_checker_puts_results->open()) {
    return false;
  }

  m_flags_defense_db = std::make_shared<ctf01d::database_file>("database_flags_defense",
    "CREATE TABLE IF NOT EXISTS flags_defense ( "
    "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "  serviceid VARCHAR(50) NOT NULL, "
    "  team_id VARCHAR(50) NOT NULL, "
    "  flag_id VARCHAR(50) NOT NULL, "
    "  flag VARCHAR(36) NOT NULL, "
    "  date_start INTEGER NOT NULL, "
    "  date_end INTEGER NOT NULL, "
    "  flag_cost INTEGER NOT NULL"
    ");",
    "flags_defense.db",
    findWsjcppEmploy<ctf01d::config>()->db_dir(),
    ctf01d::DEFAULT_DATABASE_BACKUP_FREQUENCY_IN_SECONDS
  );
  sea5kg::log::info(TAG, "Opening m_flags_defense_db");
  if (!m_flags_defense_db->open()) {
    return false;
  }

  m_flags_check_fails = std::make_shared<ctf01d::database_file>("database_flags_check_fails",
    "CREATE TABLE IF NOT EXISTS flags_check_fails ( "
    "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "  serviceid VARCHAR(50) NOT NULL, "
    "  flag_id VARCHAR(50) NOT NULL, "
    "  flag VARCHAR(36) NOT NULL, "
    "  team_id VARCHAR(50) NOT NULL, "
    "  date_start INTEGER NOT NULL, "
    "  date_end INTEGER NOT NULL, "
    "  reason VARCHAR(50) NOT NULL "
    ");",
    "flags_check_fails.db",
    findWsjcppEmploy<ctf01d::config>()->db_dir(),
    ctf01d::DEFAULT_DATABASE_BACKUP_FREQUENCY_IN_SECONDS
  );
  sea5kg::log::info(TAG, "Opening m_flags_check_fails");
  if (!m_flags_check_fails->open()) {
    return false;
  }

  m_flags_stolen = std::make_shared<ctf01d::database_file>("database_flags_stolen",
    "CREATE TABLE IF NOT EXISTS flags_stolen ( "
    "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "  serviceid VARCHAR(50) NOT NULL, "
    "  team_id VARCHAR(50) NOT NULL, "
    "  thief_team_id VARCHAR(50) NOT NULL, "
    "  flag_id VARCHAR(50) NOT NULL, "
    "  flag VARCHAR(36) NOT NULL, "
    "  date_start INTEGER NOT NULL, "
    "  date_end INTEGER NOT NULL, "
    "  date_action INTEGER NOT NULL, "
    "  flag_cost INTEGER NOT NULL "
    ");",
    "flags_stolen.db",
    findWsjcppEmploy<ctf01d::config>()->db_dir(),
    ctf01d::DEFAULT_DATABASE_BACKUP_FREQUENCY_IN_SECONDS
  );
  // TODO
  // "  INDEX(`serviceid`), "
  // "  INDEX(`serviceid`, `thief_team_id`), "
  // "  UNIQUE KEY(`serviceid`, `thief_team_id`, `flag_id`, `flag`)"
  sea5kg::log::info(TAG, "Opening m_flags_stolen");
  if (!m_flags_stolen->open()) {
    return false;
  }

    return true;
}

bool employ_database::deinit(const std::string &sName, bool bSilent) {
  sea5kg::log::info(TAG, "deinit");
  ctf01d::global_databases::shutdown_driver_sqlite3();
  return true;
}

void employ_database::insert_to_flags_checker_put_result(ctf01d::flag flag, std::string sResult) {
  std::string sQuery = "INSERT INTO flags_checker_put_results(serviceid, flag_id, flag, team_id, "
    "   date_start, date_end, result) VALUES("
    "'" + flag.service_id() + "', "
    + "'" + flag.id() + "', "
    + "'" + flag.value() + "', "
    + "'" + flag.team_id() + "', "
    + std::to_string(flag.time_start_in_milliseconds()) + ", "
    + std::to_string(flag.time_end_in_milliseconds()) + ", "
    + "'" + sResult + "'"
    + ");";
  if (!m_flags_checker_puts_results->executeQuery(sQuery)) {
    sea5kg::log::error(TAG, "Error insert " + sQuery);
  }
}

int employ_database::number_of_flags_checker_put_all_results(std::string team_id, std::string service_id) {
  return m_flags_checker_puts_results->selectSumOrCount(
    "SELECT COUNT(*) as defense FROM flags_checker_put_results "
    "WHERE serviceid = '" + service_id + "' "
    "   AND team_id = '" + team_id + "' "
    ";"
  );
}

int employ_database::number_of_flags_checker_put_success_result(std::string team_id, std::string service_id) {
  return m_flags_checker_puts_results->selectSumOrCount(
    "SELECT COUNT(*) as defense FROM flags_checker_put_results "
    "WHERE serviceid = '" + service_id + "' "
    "   AND team_id = '" + team_id + "' "
    "   AND result = 'up' "
    ";"
  );
}

void employ_database::insertToFlagsDefense(ctf01d::flag flag, int nPoints) {
  std::string sQuery = "INSERT INTO flags_defense(serviceid, team_id, flag_id, flag, "
    "   date_start, date_end, flag_cost) VALUES("
    "'" + flag.service_id() + "', "
    + "'" + flag.team_id() + "', "
    + "'" + flag.id() + "', "
    + "'" + flag.value() + "', "
    + std::to_string(flag.time_start_in_milliseconds()) + ", "
    + std::to_string(flag.time_end_in_milliseconds()) + ", "
    + std::to_string(nPoints) + " "
    + ");";

  if (!m_flags_defense_db->executeQuery(sQuery)) {
    sea5kg::log::error(TAG, "Error insert insertToFlagsDefense");
  }
}

int employ_database::number_of_flags_defense(std::string team_id, std::string service_id) {
  return m_flags_defense_db->selectSumOrCount(
    "SELECT COUNT(*) as defense FROM flags_defense "
    "WHERE serviceid = '" + service_id + "' "
    "   AND team_id = '" + team_id + "' "
    ";"
  );
}

int employ_database::sum_points_of_flags_defense(std::string team_id, std::string service_id) {
  return m_flags_defense_db->selectSumOrCount(
    "SELECT SUM(flag_cost) as points FROM flags_defense "
    "WHERE serviceid = '" + service_id + "' "
    "   AND team_id = '" + team_id + "' "
    ";"
  );
}

int employ_database::number_of_defense_flag_for_service(std::string service_id) {
  return m_flags_defense_db->selectSumOrCount(
    "SELECT COUNT(*) as cnt FROM flags_defense WHERE serviceid = '" + service_id + "'"
  );
}

void employ_database::insert_flag_check_fail(ctf01d::flag flag, std::string sReason) {
  std::string sQuery = "INSERT INTO flags_check_fails(serviceid, flag_id, flag, team_id, "
    "   date_start, date_end, reason) VALUES("
    "'" + flag.service_id() + "', "
    + "'" + flag.id() + "', "
    + "'" + flag.value() + "', "
    + "'" + flag.team_id() + "', "
    + std::to_string(flag.time_start_in_milliseconds()) + ", "
    + std::to_string(flag.time_end_in_milliseconds()) + ", "
    + "'" + sReason + "'"
    + ");";

  if (!m_flags_check_fails->executeQuery(sQuery)) {
    sea5kg::log::error(TAG, "Error insert insertToFlagsDefense");
  }
}


int employ_database::number_of_flags_stollen(std::string team_id, std::string service_id) {
  return m_flags_stolen->selectSumOrCount(
    "SELECT COUNT(*) as cnt FROM flags_stolen "
    "   WHERE serviceid = '" + service_id + "' "
    "   AND thief_team_id = '" + team_id + "' "
    ";"
  );
}

int employ_database::number_of_flags_stollen_by_victim(std::string team_id, std::string service_id) {
  return m_flags_stolen->selectSumOrCount(
    "SELECT COUNT(*) as cnt FROM flags_stolen "
    "   WHERE serviceid = '" + service_id + "' "
    "   AND team_id = '" + team_id + "' "
    ";"
  );
}

int employ_database::sum_points_of_flags_stolen(std::string team_id, std::string service_id) {
  return m_flags_stolen->selectSumOrCount(
    "SELECT SUM(flag_cost) as points FROM flags_stolen "
    "WHERE serviceid = '" + service_id + "' "
    "   AND thief_team_id = '" + team_id + "' "
    ";"
  );
}

int employ_database::number_of_stolen_flags_for_service(std::string service_id) {
  return m_flags_stolen->selectSumOrCount(
    "SELECT COUNT(*) as cnt FROM flags_stolen WHERE serviceid = '" + service_id + "'"
  );
}

std::pair<std::string, long> employ_database::get_first_blood_from_stolen_flags_for_service(std::string service_id) {
  std::string sQuery = "SELECT thief_team_id, date_action FROM flags_stolen WHERE serviceid = '" + service_id + "' LIMIT 1";
  std::pair<std::string, long> pairRet;
  pairRet.first = "?";
  pairRet.second = 0;
  auto rows = m_flags_stolen->selectRows(sQuery);
  if (rows == nullptr) {
    sea5kg::log::error(TAG, "Error select get_first_blood_from_stolen_flags_for_service " + sQuery);
    return pairRet;
  }
  if (rows->next()) {
    pairRet.first = rows->getString(0);
    pairRet.second = rows->getLong(1);
  }
  return pairRet;
}

void employ_database::insert_to_flags_stolen(ctf01d::flag flag, std::string team_id, int nPoints, long date_action, int victim_place_in_scoreboard, int thief_place_in_scoreboard) {
  // TODO
  // victim_place_in_scoreboard
  // thief_place_in_scoreboard
  std::string sQuery = "INSERT INTO flags_stolen(serviceid, team_id, thief_team_id, flag_id, flag,"
    "   date_start, date_end, date_action, flag_cost) VALUES("
    "'" + flag.service_id() + "', "
    + "'" + flag.team_id() + "', "
    + "'" + team_id + "', "
    + "'" + flag.id() + "', "
    + "'" + flag.value() + "', "
    + std::to_string(flag.time_start_in_milliseconds()) + ", "
    + std::to_string(flag.time_end_in_milliseconds()) + ", "
    + std::to_string(date_action) + ", "
    + std::to_string(nPoints) + " "
    + ");";

  if (!m_flags_stolen->executeQuery(sQuery)) {
    sea5kg::log::error(TAG, "Error insert insertToFlagsDefense");
  }
}

bool employ_database::is_already_stole(ctf01d::flag flag, std::string team_id) {
  int nRet = m_flags_stolen->selectSumOrCount(
    "SELECT COUNT(*) as cnt FROM flags_stolen "
    " WHERE serviceid = '" + flag.service_id() + "' "
    "   AND thief_team_id = '" + team_id + "'"
    "   AND flag_id = '" + flag.id() + "'"
    "   AND flag = '" + flag.value() + "'"
  );
  return nRet > 0;
}

bool employ_database::is_somebody_stole(ctf01d::flag flag) {
  int nRet = m_flags_stolen->selectSumOrCount(
    "SELECT COUNT(*) as cnt FROM flags_stolen "
    " WHERE serviceid = '" + flag.service_id() + "' "
    "   AND team_id = '" + flag.team_id() + "'"
    "   AND flag_id = '" + flag.id() + "'"
    "   AND flag = '" + flag.value() + "'"
  );
  return nRet > 0;
}
