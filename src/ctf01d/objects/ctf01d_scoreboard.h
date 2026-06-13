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

#pragma once

#include "ctf01d/include/ctf01d_alive_flags.h"
#include "ctf01d/include/ctf01d_activities.h"
#include "ctf01d/include/ctf01d_database.h"
#include "ctf01d/employees/employ_scoreboard.h"
#include <optional>
#include <string>
#include <json.hpp>
#include "ctf01d_formulas_for_points.h"
#include "ctf01d_team_status_row.h"
#include "ctf01d_var.h"

namespace ctf01d {

class scoreboard {
public:
  scoreboard(
    bool random,
    int game_start_in_seconds,
    int game_end_in_seconds,
    int game_coffee_break_start_in_seconds,
    int game_coffee_break_end_in_seconds
  );

  void set_service_status(const std::string &team_id, const std::string &sServiceId, const std::string &sStatus);
  void insert_flag_attempt(const std::string &thief_team_id, const std::string &flag_value, const std::string &request_ip);
  void init_state_from_storage();

  // Returns flag points on success; std::nullopt if this team has already
  // stolen the flag (dedup check happens under the same lock as the insert
  // so concurrent submissions can't double-credit).
  std::optional<int> increment_attack_score(const ctf01d::flag &flag, const std::string &team_id);
  void increment_defense_score(const ctf01d::flag &flag);
  void increment_flags_putted_and_service_up(const ctf01d::flag &flag);
  void insert_flag_put_fail(const ctf01d::flag &flag, const std::string &service_status, const std::string &description_status);
  // void update_points(const std::string &team_id, const std::string &service_id);
  std::string service_status(const std::string &team_id, const std::string &service_id);
  const nlohmann::json &to_json();

private:
  std::string TAG;
  ctf01d::alive_flags *m_alive_flags;
  ctf01d::activities *m_activities;
  ctf01d::database *m_database;
  std::shared_ptr<ctf01d::var_int> m_flag_cost_in_points;
  int m_game_start_in_seconds;
  int m_game_end_in_seconds;
  int m_game_coffee_break_start_in_seconds;
  int m_game_coffee_break_end_in_seconds;
  int m_team_count;

  void sort_places(); // TODO merge this function with update costs
  void update_services_statistics();

  // TODO move to employ scoreboard
  std::map<std::string, ctf01d::service_statistics *> m_service_statistics;
  int m_all_defense_flags;

  std::string random_service_status();
  bool m_random;
  // TODO shared ptr and move to employ scoreboard
  std::map<std::string, ctf01d::team_status_row *> m_teams_statuses;

  std::mutex m_mutex_scoreboard;
  nlohmann::json m_scoreboard; // prepare data for scoreboard
  void init_json_scoreboard();
  void update_json_scoreboard();

  std::shared_ptr<ctf01d::formulas_for_points> m_formulas;
};

} // namespace ctf01d
