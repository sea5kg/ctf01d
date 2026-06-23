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

#include "employ_scoreboard.h"
#include <wsjcpp_core.h>
#include "ctf01d/include/ctf01d_config.h"
#include <sea5kg_logger.h>
#include <cmath>
#include <stdio.h>
#include <map>
#include <mutex>
#include <vector>

REGISTRY_WSJCPP_EMPLOY(employ_scoreboard)

employ_scoreboard::employ_scoreboard()
: WsjcppEmployBase({ employ_scoreboard::name() }, { ctf01d::config::name(), ctf01d::database::name() }) {
  TAG = employ_scoreboard::name();
}

bool employ_scoreboard::init(const std::string &sName, bool bSilent) {
  if (!init_services_stats()) {
    return false;
  }

  // scoreboard
  auto config = findWsjcppEmploy<ctf01d::config>();
  m_scoreboard = std::make_shared<ctf01d::scoreboard>(
    config->scoreboard_random(),
    config->game_start_utc_in_seconds(),
    config->game_end_utc_in_seconds(),
    config->game_coffee_break_start_utc_in_seconds(),
    config->game_coffee_break_end_utc_in_seconds()
  );

  sea5kg::log::info(TAG, "Restoring states from storage...");
  m_scoreboard->init_state_from_storage();
  sea5kg::log::ok(TAG, "Restored state from storage.");
  return true;
}

bool employ_scoreboard::deinit(const std::string &sName, bool bSilent) {
  sea5kg::log::info(TAG, "deinit");
  return true;
}

bool employ_scoreboard::init_services_stats() {
  std::lock_guard<std::mutex> lock(m_mutex_services_statistics);
  m_services_statistics.clear();

  auto config = findWsjcppEmploy<ctf01d::config>();
  const std::vector<ctf01d::team_config> &teams_conf = config->teams();
  const std::vector<ctf01d::service_config> &services_conf = config->services();

  // keep the list of the services ids
  for (unsigned int i = 0; i < services_conf.size(); i++) {
    std::string service_id = services_conf[i].id();
    m_services_statistics[service_id] = std::make_shared<ctf01d::service_statistics>(service_id);
  }
  return true;
}

void employ_scoreboard::set_service_status(const std::string &team_id, const std::string &service_id, const std::string &status) {
  m_scoreboard->insert_flag_attempt(team_id, service_id, status);
}

void employ_scoreboard::insert_flag_attempt(const std::string &thief_team_id, const std::string &flag_value, const std::string &request_ip) {
  m_scoreboard->insert_flag_attempt(thief_team_id, flag_value, request_ip);
}

void employ_scoreboard::init_state_from_storage() {
  m_scoreboard->init_state_from_storage();
}

std::optional<int> employ_scoreboard::increment_attack_score(const ctf01d::flag &flag, const std::string &team_id) {
  return m_scoreboard->increment_attack_score(flag, team_id);
}

void employ_scoreboard::increment_defense_score(const ctf01d::flag &flag) {
  m_scoreboard->increment_defense_score(flag);
}

void employ_scoreboard::increment_flags_putted_and_service_up(const ctf01d::flag &flag) {
  m_scoreboard->increment_flags_putted_and_service_up(flag);
}

void employ_scoreboard::insert_flag_put_fail(const ctf01d::flag &flag, const std::string &service_status, const std::string &description_status) {
  m_scoreboard->insert_flag_put_fail(flag, service_status, description_status);
}

std::string employ_scoreboard::service_status(const std::string &team_id, const std::string &service_id) {
  return m_scoreboard->service_status(team_id, service_id);
}


const nlohmann::json &employ_scoreboard::to_json() {
  return m_scoreboard->to_json();
}
