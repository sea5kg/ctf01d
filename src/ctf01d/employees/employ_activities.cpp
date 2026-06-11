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
#include "ctf01d/include/ctf01d_globals.h"
#include "ctf01d/objects/ctf01d_database_file.h"
#include "ctf01d/objects/ctf01d_flag.h"
#include "ctf01d/include/ctf01d_activities.h"
#include "ctf01d/employees/employ_config.h"
#include "ctf01d/utils/ctf01d_logger.h"
#include "ctf01d/utils/ctf01d_time_measurer.h"
#include <wsjcpp_core.h>
#include <fstream>
#include <cstring>

// ---------------------------------------------------------------------
// EmployAliveFlags definition

class employ_activities : public WsjcppEmployBase, public ctf01d::activities {
public:
  employ_activities();

  // WsjcppEmployBase
  virtual bool init(const std::string &name, bool silent) override;
  virtual bool deinit(const std::string &name, bool silent) override;

  // ctf01d::activities
  virtual void update_scoreboard(nlohmann::json &scoreboard) override;
  virtual void insert_flag_attempt(
    const std::string &thief_team_id,
    const std::string &flag_value,
    const std::string &request_ip,
    nlohmann::json &scoreboard
  ) override;

private:
  bool init_flags_attempts_db();
  bool init_flags_attempts_snapshots_db();

  std::string TAG;

  // cache
  int m_all_activities_send_flag;
  std::mutex m_mutex_teams_activities_send_flag;
  std::map<std::string, int> m_teams_activities_send_flag;

  // db
  std::mutex m_mutex_flags_attempts_db;
  std::shared_ptr<Ctf01dDatabaseFile> m_flags_attempts_db;
  std::shared_ptr<Ctf01dDatabaseFile> m_flags_attempts_snapshots_db;
};

// ---------------------------------------------------------------------
// EmployAliveFlags implementation

REGISTRY_WSJCPP_EMPLOY(employ_activities)

employ_activities::employ_activities()
: WsjcppEmployBase({ ctf01d::activities::name() }, { EmployConfig::name(), EmployDatabase::name() }) {
  TAG = "employ_activities";
  m_all_activities_send_flag = 0;
  m_flags_attempts_db = nullptr;
}

bool employ_activities::init(const std::string &name, bool silent) {
  ctf01d::log::info(TAG, "init");
  std::lock_guard<std::mutex> lock(m_mutex_flags_attempts_db);
  
  m_all_activities_send_flag = 0;
  
  if (!init_flags_attempts_db()) {
    return false;
  }

  if (!init_flags_attempts_snapshots_db()) {
    return false;
  }

  {
    auto config = findWsjcppEmploy<EmployConfig>();
    ctf01d::time_measurer measurer("restore activities from database");
    std::lock_guard<std::mutex> lock(m_mutex_teams_activities_send_flag);
    for (unsigned int i = 0; i < config->teamsConf().size(); i++) {
      const ctf01d::team_config &team_config = config->teamsConf()[i];
      int flag_attempts_sum = m_flags_attempts_db->selectSumOrCount(
        "SELECT COUNT(*) FROM flags_attempts"
        "  WHERE "
        "    team_id = '" + team_config.id() + "'"
        "    AND dt >= " + std::to_string(long(config->gameStartUTCInSec())*1000) + " "
        "    AND dt <= " + std::to_string(long(config->gameEndUTCInSec())*1000) + " "
      );
      m_all_activities_send_flag += flag_attempts_sum;
      m_teams_activities_send_flag[team_config.id()] = flag_attempts_sum;
    }
  }
  return true;
}

bool employ_activities::deinit(const std::string &name, bool silent) {
  ctf01d::log::info(TAG, "deinit");
  return true;
}

void employ_activities::update_scoreboard(nlohmann::json &scoreboard) {
  ctf01d::log::info(TAG, "Updating activities in scoreboard...");
  std::lock_guard<std::mutex> lock(m_mutex_teams_activities_send_flag);
  std::map<std::string, int>::iterator it;
  for (it = m_teams_activities_send_flag.begin(); it != m_teams_activities_send_flag.end(); it++) {
    const std::string &team_id = it->first;
    // only if team_id exists in json
    ctf01d::log::info(TAG, "Update for team " + team_id);
    // TODO recalculate summary if some team missing
    if (scoreboard["scoreboard"].contains(team_id)) {
      ctf01d::log::info(TAG, "Updated for team " + team_id + " " + std::to_string(it->second));
      scoreboard["scoreboard"][team_id][ctf01d::JSON_FIELD_TRIES] = it->second;
    }
  }
  scoreboard[ctf01d::JSON_FIELD_SUMMARY_ACTIVITIES] = m_all_activities_send_flag;
  ctf01d::log::info(TAG, "m_all_activities_send_flag: " + std::to_string(m_all_activities_send_flag));
}

// void employ_activities::increment_activity_send_flag(const std::string &team_id, nlohmann::json &scoreboard) {

// }

// TODO result of send_flag (error code or success + elapsed time)
void employ_activities::insert_flag_attempt(
  const std::string &thief_team_id,
  const std::string &flag_value,
  const std::string &request_ip,
  nlohmann::json &scoreboard
) {
  {
    std::lock_guard<std::mutex> lock(m_mutex_flags_attempts_db);
    std::string sQuery = "INSERT INTO flags_attempts(flag, team_id, request_ip, dt) "
      " VALUES('" + flag_value + "', '" + thief_team_id + "', '" + request_ip + "', " + std::to_string(WsjcppCore::getCurrentTimeInMilliseconds()) + ");";

    if (!m_flags_attempts_db->executeQuery(sQuery)) {
      ctf01d::log::throw_err(TAG, "Error insert attempt");
    }
  }

  {
    std::lock_guard<std::mutex> lock(m_mutex_teams_activities_send_flag);
    m_all_activities_send_flag++;
    int new_team_activity = 0;

    // collect team ids in realtime
    auto it = m_teams_activities_send_flag.find(thief_team_id);
    if (it != m_teams_activities_send_flag.end()) {
      new_team_activity = it->second + 1;
    } else {
      new_team_activity = 1;
    }
    m_teams_activities_send_flag[thief_team_id] = new_team_activity;

    // only if thief_team_id exists in json  
    if (scoreboard["scoreboard"].contains(thief_team_id)) {
      scoreboard["scoreboard"][thief_team_id][ctf01d::JSON_FIELD_TRIES] = new_team_activity;
    }

    scoreboard[ctf01d::JSON_FIELD_SUMMARY_ACTIVITIES] = m_all_activities_send_flag;
  }
}

bool employ_activities::init_flags_attempts_db() {
  m_flags_attempts_db = std::make_shared<Ctf01dDatabaseFile>("flags_attempts.db",
    "CREATE TABLE IF NOT EXISTS flags_attempts ( "
    "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "  flag VARCHAR(1024) NOT NULL, "
    "  team_id VARCHAR(50) NOT NULL, "
    "  request_ip VARCHAR(50) NOT NULL, "
    "  dt INTEGER NOT NULL"
    ");"
    // TODO result of send_flag (error code or success + elapsed time)
  );
  ctf01d::log::info(TAG, "Opening flags_attempts.db");
  if (!m_flags_attempts_db->open()) {
    return false;
  }

  m_flags_attempts_db->executeQuery("CREATE INDEX IF NOT EXISTS  idx_dt ON flags_attempts(dt);");
  return true;
}

bool employ_activities::init_flags_attempts_snapshots_db()
{
  m_flags_attempts_snapshots_db = std::make_shared<Ctf01dDatabaseFile>("flags_attempts_snapshots.db",
    "CREATE TABLE IF NOT EXISTS flags_attempts_snapshots ( "
    "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "  team_id VARCHAR(50) NOT NULL, "
    "  snapshot_dt INTEGER NOT NULL, "
    "  total_attempts INTEGER NOT NULL, " // TODO
    "  total_success INTEGER NOT NULL, " // TODO
    "  total_failed INTEGER NOT NULL " // TODO
    ");"
    // TODO result of send_flag (error code or success + elapsed time)
  );
  ctf01d::log::info(TAG, "Opening flags_attempts_snapshots.db");
  if (!m_flags_attempts_snapshots_db->open()) {
    return false;
  }
  return true;
}