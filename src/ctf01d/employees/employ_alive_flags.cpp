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
#include <string>
#include "ctf01d/objects/ctf01d_database_file.h"
#include "ctf01d/objects/ctf01d_flag.h"
#include "ctf01d/include/ctf01d_alive_flags.h"
#include "ctf01d/employees/employ_config.h"
#include "ctf01d/utils/ctf01d_logger.h"
#include <wsjcpp_core.h>
#include <fstream>
#include <cstring>

// ---------------------------------------------------------------------
// EmployAliveFlags definition

class EmployAliveFlags : public WsjcppEmployBase, public ctf01d::alive_flags {
public:
  EmployAliveFlags();

  // WsjcppEmployBase
  virtual bool init(const std::string &name, bool silent) override;
  virtual bool deinit(const std::string &name, bool silent) override;

  // ctf01d::alive_flags
  virtual bool insert_alive_flag(const ctf01d::flag &flag) override;
  virtual std::vector<ctf01d::flag> outdated_alive_flags(const std::string &team_id, const std::string &service_id) override;
  virtual bool find_alive_flag(const std::string &flag_value, ctf01d::flag &flag) override;
  virtual void remove_alive_flag(const ctf01d::flag &flag) override;
  virtual int count_alive_flags() override;

private:
  std::string TAG;

  std::vector<ctf01d::flag> get_from_db_alive_flags();

  std::mutex m_mutex_alive_flags;
  std::map<std::string, ctf01d::flag> m_alive_flags_cache;
  std::shared_ptr<ctf01d::database_file> m_alive_flags_db;
};

// ---------------------------------------------------------------------
// EmployAliveFlags implementation

REGISTRY_WSJCPP_EMPLOY(EmployAliveFlags)

EmployAliveFlags::EmployAliveFlags()
: WsjcppEmployBase({ ctf01d::alive_flags::name() }, { EmployConfig::name(), EmployDatabase::name() }) {
  TAG = "EmployAliveFlags";
  m_alive_flags_db = nullptr;
}

bool EmployAliveFlags::init(const std::string &name, bool silent) {
  ctf01d::log::info(TAG, "init");
  std::lock_guard<std::mutex> lock(m_mutex_alive_flags);

  m_alive_flags_db = std::make_shared<ctf01d::database_file>("alive_flags.db",
    "CREATE TABLE IF NOT EXISTS alive_flags ( "
    "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "  service_id VARCHAR(50) NOT NULL, "
    "  flag_id VARCHAR(50) NOT NULL, "
    "  flag VARCHAR(36) NOT NULL, "
    "  team_id VARCHAR(50) NOT NULL, "
    "  date_start INTEGER NOT NULL, "
    "  date_end INTEGER NOT NULL "
    ");"
  );
  ctf01d::log::info(TAG, "Opening alive_flags.db");
  if (!m_alive_flags_db->open()) {
      return false;
  }

  // load alive flags
  ctf01d::log::info(TAG, "Loading alive flags...");
  std::vector<ctf01d::flag> alive_flags = get_from_db_alive_flags();
  for (unsigned int i = 0; i < alive_flags.size(); i++) {
    // TODO check service_id and team_id
    ctf01d::flag flag = alive_flags[i];
    m_alive_flags_cache[flag.getValue()] = flag;
    ctf01d::log::info(TAG, "Loaded flag from previous session flags_live: id = " + flag.getId() + ", value = " + flag.getValue());
  }

  return true;
}

bool EmployAliveFlags::deinit(const std::string &name, bool silent) {
  ctf01d::log::info(TAG, "deinit");
  return true;
}

bool EmployAliveFlags::insert_alive_flag(const ctf01d::flag &flag) {
  std::lock_guard<std::mutex> lock(m_mutex_alive_flags);
  std::map<std::string, ctf01d::flag>::iterator it;
  it = m_alive_flags_cache.find(flag.getValue());
  if (it != m_alive_flags_cache.end()) {
    ctf01d::log::err(TAG, flag.getValue() + " - flag already exists");
    return false;
  }
  m_alive_flags_cache[flag.getValue()] = flag;

  std::string sQuery = "INSERT INTO alive_flags(service_id, flag_id, flag, team_id, "
    "   date_start, date_end) VALUES("
    "'" + flag.getServiceId() + "', "
    + "'" + flag.getId() + "', "
    + "'" + flag.getValue() + "', "
    + "'" + flag.getTeamId() + "', "
    + std::to_string(flag.getTimeStartInMs()) + ", "
    + std::to_string(flag.getTimeEndInMs())
    + ");";
  if (!m_alive_flags_db->executeQuery(sQuery)) {
    ctf01d::log::err(TAG, "Error insert insertToFlagLive");
  }
  return true;
}

std::vector<ctf01d::flag> EmployAliveFlags::outdated_alive_flags(const std::string &team_id, const std::string &service_id) {
  std::lock_guard<std::mutex> lock(m_mutex_alive_flags);
  std::vector<ctf01d::flag> vResult;
  long current_time = WsjcppCore::getCurrentTimeInMilliseconds();
  std::map<std::string,ctf01d::flag>::iterator it;
  for (it = m_alive_flags_cache.begin(); it != m_alive_flags_cache.end(); it++) {
    ctf01d::flag flag = it->second;
    if (flag.getTeamId() == team_id
      && flag.getServiceId() == service_id
      && flag.getTimeEndInMs() < current_time
    ) {
      vResult.push_back(flag);
    }
  }
  return vResult;
}

bool EmployAliveFlags::find_alive_flag(const std::string &sFlagValue, ctf01d::flag &flag) {
  std::lock_guard<std::mutex> lock(m_mutex_alive_flags);
  std::map<std::string,ctf01d::flag>::iterator it = m_alive_flags_cache.find(sFlagValue);
  if (it != m_alive_flags_cache.end()) {
    flag.copyFrom(it->second);
    return true;
  }
  return false;
}

void EmployAliveFlags::remove_alive_flag(const ctf01d::flag &flag) {
  std::lock_guard<std::mutex> lock(m_mutex_alive_flags);
  std::map<std::string,ctf01d::flag>::iterator it;
  it = m_alive_flags_cache.find(flag.getValue());
  if (it != m_alive_flags_cache.end()) {
    m_alive_flags_cache.erase(it);

    std::string sQuery = "DELETE FROM alive_flags WHERE flag = '" + flag.getValue() + "';";
    if (!m_alive_flags_db->executeQuery(sQuery)) {
      ctf01d::log::err(TAG, "Error delete deleteFlagLive");
    }
  } else {
    ctf01d::log::warn(TAG, flag.getValue() + " - flag did not exists");
  }
}

int EmployAliveFlags::count_alive_flags() {
  std::lock_guard<std::mutex> lock(m_mutex_alive_flags);
  return static_cast<int>(m_alive_flags_cache.size());
}

std::vector<ctf01d::flag> EmployAliveFlags::get_from_db_alive_flags() {
  // long nCurrentTime = WsjcppCore::getCurrentTimeInMilliseconds();
  auto config = findWsjcppEmploy<EmployConfig>();

  std::string sQuery =
    "SELECT flag_id, service_id, team_id, flag, date_start, date_end "
    "FROM alive_flags "
    "WHERE "
    "   date_start > " + std::to_string(long(config->gameStartUTCInSec())*1000) + " "
    "   AND date_end < " + std::to_string(long(config->gameEndUTCInSec())*1000) + " "
    ";";

  std::vector<ctf01d::flag> vResult;
  auto rows = m_alive_flags_db->selectRows(sQuery);
  if (rows == nullptr) {
    ctf01d::log::err(TAG, "Error select listOfLiveFlags " + sQuery);
    return vResult;
  }
  int nCounter = 0;
  while (rows->next()) {
    nCounter++;
    ctf01d::flag flag;
    std::string sFlagId = rows->getString(0);
    flag.setId(sFlagId);
    flag.setServiceId(rows->getString(1));
    flag.setTeamId(rows->getString(2));
    std::string sFlagValue = rows->getString(3);
    flag.setValue(sFlagValue);
    flag.setTimeStartInMs(rows->getLong(4));
    flag.setTimeEndInMs(rows->getLong(5));
    vResult.push_back(flag);
  }
  return vResult;
}
