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

#include "ctf01d/include/ctf01d_alive_flags.h"
#include "ctf01d/include/ctf01d_config.h"
#include "ctf01d/include/ctf01d_database.h"
#include "ctf01d/include/ctf01d_globals.h"
#include "ctf01d/objects/ctf01d_flag.h"
#include <cstring>
#include <fstream>
#include <sea5kg_logger.h>
#include <sea5kg_sqlite3_wrapper.h>
#include <string>
#include <wsjcpp_core.h>
#include <wsjcpp_employees.h>

class EmployAliveFlags : public WsjcppEmployBase, public ctf01d::alive_flags {
public:
  EmployAliveFlags();

  // WsjcppEmployBase
  virtual bool init(const std::string &name, bool silent) override;
  virtual bool deinit(const std::string &name, bool silent) override;

  // ctf01d::alive_flags
  virtual bool insert_alive_flag(const ctf01d::flag &flag) override;
  virtual std::vector<ctf01d::flag>
  outdated_alive_flags(const std::string &team_id, const std::string &service_id) override;
  virtual bool find_alive_flag(const std::string &flag_value, ctf01d::flag &flag) override;
  virtual void remove_alive_flag(const ctf01d::flag &flag) override;
  virtual int count_alive_flags() override;

private:
  std::string TAG;

  std::vector<ctf01d::flag> get_from_db_alive_flags();

  std::mutex m_mutex_alive_flags;
  std::map<std::string, ctf01d::flag> m_alive_flags_cache;
  std::shared_ptr<sea5kg::sqlite3_wrapper::database_file> m_alive_flags_db;
};

// ---------------------------------------------------------------------
// EmployAliveFlags implementation

REGISTRY_WSJCPP_EMPLOY(EmployAliveFlags)

CLASS_DATABASE_UPDATE_BEGIN(database_alive_flags, initial, v001, "Init table alive_flags") {
  // IF NOT EXISTS
  return db->execute_query(
    "CREATE TABLE IF NOT EXISTS alive_flags ( "
    "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "  service_id VARCHAR(50) NOT NULL, "
    "  flag_id VARCHAR(50) NOT NULL, "
    "  flag VARCHAR(36) NOT NULL, "
    "  team_id VARCHAR(50) NOT NULL, "
    "  date_start INTEGER NOT NULL, "
    "  date_end INTEGER NOT NULL "
    ");",
    error
  );
}
CLASS_DATABASE_UPDATE_END()

EmployAliveFlags::EmployAliveFlags()
    : WsjcppEmployBase({ctf01d::alive_flags::name()}, {ctf01d::config::name(), ctf01d::database::name()}) {
  TAG = "EmployAliveFlags";
  m_alive_flags_db = nullptr;
}

bool EmployAliveFlags::init(const std::string &name, bool silent) {
  sea5kg::log::info(TAG, "init");
  std::lock_guard<std::mutex> lock(m_mutex_alive_flags);

  m_alive_flags_db = std::make_shared<sea5kg::sqlite3_wrapper::database_file>(
    "database_alive_flags",
    "",
    findWsjcppEmploy<ctf01d::config>()->db_dir(),
    "alive_flags.db",
    ctf01d::DEFAULT_DATABASE_BACKUP_FREQUENCY_IN_SECONDS
  );
  sea5kg::log::info(TAG, "Opening alive_flags.db");
  std::string error;
  if (!m_alive_flags_db->open(error)) {
    sea5kg::log::critical(TAG, "Problem with open database. Error: " + error);
    return false;
  }

  // load alive flags
  sea5kg::log::info(TAG, "Loading alive flags...");
  std::vector<ctf01d::flag> alive_flags = get_from_db_alive_flags();
  for (unsigned int i = 0; i < alive_flags.size(); i++) {
    // TODO check service_id and team_id
    ctf01d::flag flag = alive_flags[i];
    m_alive_flags_cache[flag.value()] = flag;
    sea5kg::log::info(
      TAG, "Loaded flag from previous session flags_live: id = " + flag.id() + ", value = " + flag.value()
    );
  }

  return true;
}

bool EmployAliveFlags::deinit(const std::string &name, bool silent) {
  sea5kg::log::info(TAG, "deinit");
  return true;
}

bool EmployAliveFlags::insert_alive_flag(const ctf01d::flag &flag) {
  std::lock_guard<std::mutex> lock(m_mutex_alive_flags);
  std::map<std::string, ctf01d::flag>::iterator it;
  it = m_alive_flags_cache.find(flag.value());
  if (it != m_alive_flags_cache.end()) {
    sea5kg::log::error(TAG, flag.value() + " - flag already exists");
    return false;
  }
  m_alive_flags_cache[flag.value()] = flag;

  std::string sQuery = "INSERT INTO alive_flags(service_id, flag_id, flag, team_id, "
                       "   date_start, date_end) VALUES("
                       "'" +
                       flag.service_id() + "', " + "'" + flag.id() + "', " + "'" + flag.value() + "', " + "'" +
                       flag.team_id() + "', " + std::to_string(flag.time_start_in_milliseconds()) + ", " +
                       std::to_string(flag.time_end_in_milliseconds()) + ");";
  std::string error;
  if (!m_alive_flags_db->execute_query(sQuery, error)) {
    sea5kg::log::error(TAG, error);
  }
  return true;
}

std::vector<ctf01d::flag>
EmployAliveFlags::outdated_alive_flags(const std::string &team_id, const std::string &service_id) {
  std::lock_guard<std::mutex> lock(m_mutex_alive_flags);
  std::vector<ctf01d::flag> vResult;
  long current_time = WsjcppCore::getCurrentTimeInMilliseconds();
  std::map<std::string, ctf01d::flag>::iterator it;
  for (it = m_alive_flags_cache.begin(); it != m_alive_flags_cache.end(); it++) {
    ctf01d::flag flag = it->second;
    if (
      flag.team_id() == team_id && flag.service_id() == service_id && flag.time_end_in_milliseconds() < current_time
    ) {
      vResult.push_back(flag);
    }
  }
  return vResult;
}

bool EmployAliveFlags::find_alive_flag(const std::string &sFlagValue, ctf01d::flag &flag) {
  std::lock_guard<std::mutex> lock(m_mutex_alive_flags);
  std::map<std::string, ctf01d::flag>::iterator it = m_alive_flags_cache.find(sFlagValue);
  if (it != m_alive_flags_cache.end()) {
    flag.copy_from(it->second);
    return true;
  }
  return false;
}

void EmployAliveFlags::remove_alive_flag(const ctf01d::flag &flag) {
  std::lock_guard<std::mutex> lock(m_mutex_alive_flags);
  std::map<std::string, ctf01d::flag>::iterator it;
  it = m_alive_flags_cache.find(flag.value());
  if (it != m_alive_flags_cache.end()) {
    m_alive_flags_cache.erase(it);

    std::string sQuery = "DELETE FROM alive_flags WHERE flag = '" + flag.value() + "';";
    std::string error;
    if (!m_alive_flags_db->execute_query(sQuery, error)) {
      sea5kg::log::error(TAG, error);
    }
  } else {
    sea5kg::log::warning(TAG, flag.value() + " - flag did not exists");
  }
}

int EmployAliveFlags::count_alive_flags() {
  std::lock_guard<std::mutex> lock(m_mutex_alive_flags);
  return static_cast<int>(m_alive_flags_cache.size());
}

std::vector<ctf01d::flag> EmployAliveFlags::get_from_db_alive_flags() {
  // long nCurrentTime = WsjcppCore::getCurrentTimeInMilliseconds();
  auto config = findWsjcppEmploy<ctf01d::config>();

  std::string sQuery = "SELECT flag_id, service_id, team_id, flag, date_start, date_end "
                       "FROM alive_flags "
                       "WHERE "
                       "   date_start > " +
                       std::to_string(long(config->game_start_utc_in_seconds()) * 1000) +
                       " "
                       "   AND date_end < " +
                       std::to_string(long(config->game_end_utc_in_seconds()) * 1000) +
                       " "
                       ";";

  std::vector<ctf01d::flag> vResult;
  std::string error;
  auto rows = m_alive_flags_db->select_rows(sQuery, error);
  if (rows == nullptr) {
    sea5kg::log::critical(TAG, "Error select listOfLiveFlags. " + error);
    return vResult;
  }
  int nCounter = 0;
  while (rows->next()) {
    nCounter++;
    ctf01d::flag flag;
    std::string flag_id = rows->as_string(0);
    flag.set_id(flag_id);
    flag.set_service_id(rows->as_string(1));
    flag.set_team_id(rows->as_string(2));
    std::string flag_value = rows->as_string(3);
    flag.set_value(flag_value);
    flag.set_time_start_in_milliseconds(rows->as_long(4));
    flag.set_time_end_in_milliseconds(rows->as_long(5));
    vResult.push_back(flag);
  }
  return vResult;
}
