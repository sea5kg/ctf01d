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

#include "ctf01d_service_statistics.h"

namespace ctf01d {

service_statistics::service_statistics(const std::string &service_id) {
  TAG = "service_statistics-" + service_id;
  m_service_id = service_id;
  m_flags_stolen = 0;
  m_flags_defense = 0;
  m_first_blood_team_id = "?";
  m_first_blood_time_in_seconds = 0;
}

int service_statistics::flags_stolen() const {
  return m_flags_stolen;
}

void service_statistics::do_increment_flags_stolen() {
  m_flags_stolen++;
}

void service_statistics::set_flags_stolen(int stolen_flags) {
  m_flags_stolen = stolen_flags;
}

int service_statistics::flags_defense() const {
  return m_flags_defense;
}

void service_statistics::do_increment_flags_defense() {
  m_flags_defense++;
}

void service_statistics::set_flags_defense(int flags_defense) {
  m_flags_defense = flags_defense;
}

int service_statistics::first_blood_time_in_seconds() const {
  return m_first_blood_time_in_seconds;
}

std::string service_statistics::first_blood_team_id() const {
  return m_first_blood_team_id;
}

void service_statistics::update_scoreboard(nlohmann::json &scoreboard) {
  scoreboard["s_sta"][m_service_id]["af_att"] = m_flags_stolen;
  scoreboard["s_sta"][m_service_id]["af_def"] = m_flags_defense;
  scoreboard["s_sta"][m_service_id]["first_blood"] = m_first_blood_team_id;
  scoreboard["s_sta"][m_service_id]["first_blood_ts"] = m_first_blood_time_in_seconds;
}

void service_statistics::set_first_blood_team_id(const std::string &team_id, long date_action) {
  m_first_blood_team_id = team_id;
  m_first_blood_time_in_seconds = date_action / 1000;
}

} // namespace ctf01d