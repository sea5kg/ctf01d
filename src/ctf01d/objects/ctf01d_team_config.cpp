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

#include "ctf01d_team_config.h"
#include "ctf01d/include/ctf01d_globals.h"
#include <sea5kg_logger.h>
#include <wsjcpp_core.h>
#include <vector>
#include <algorithm>

namespace ctf01d {

team_config::team_config() {
  m_id = ctf01d::var_string::create({yaml_keys::ID}, "", m_vars);
  // normal, red, blue, guest, inactive, disqualified
  m_type = ctf01d::var_string::create({yaml_keys::TYPE}, yaml_keys::TYPE_NORMAL, m_vars); // TODO var_types
  m_name = ctf01d::var_string::create({yaml_keys::NAME}, "", m_vars);
  m_description = ctf01d::var_string::create({yaml_keys::DESCRIPTION}, "", m_vars);
  m_active = ctf01d::var_bool::create({yaml_keys::ACTIVE}, true, m_vars); // TODO rename enabled
  m_logo = ctf01d::var_file::create({yaml_keys::LOGO}, "", "", m_vars);
  m_logo_big = ctf01d::var_file::create({yaml_keys::LOGO_BIG}, "", "", m_vars);
  m_ip_or_host = ctf01d::var_ip_or_host::create({yaml_keys::IP_OR_HOST}, m_vars);
  m_updated_time = 0;
}

bool team_config::read(WsjcppYamlCursor &cursor, const std::string &work_dir, std::string &err) {
  m_work_dir = work_dir;
  m_logo->set_root_dir(m_work_dir);
  m_logo_big->set_root_dir(m_work_dir);
  if (!m_vars.read(cursor, err)) {
    return false;
  }
  // check type
  static const std::vector<std::string> allowed_types = {
    yaml_keys::TYPE_NORMAL,
    yaml_keys::TYPE_RED,
    yaml_keys::TYPE_BLUE,
    yaml_keys::TYPE_QUEST,
    yaml_keys::TYPE_DISQUALIFIED,
  };
  if (std::find(allowed_types.begin(), allowed_types.end(), m_type->value()) == allowed_types.end()) {
    err = "Didn't allowed team.type: '" + m_type->value() + "'";
    sea5kg::log::err(TAG, err);
    return false;
  }

  return true;
}

std::string team_config::id() const {
  return m_id->value();
}

std::string team_config::name() const {
  return m_name->value();
}

std::string team_config::description() const {
  return m_description->value();
}

void team_config::set_ip_or_host_prefix(const std::string &val) {
  m_ip_or_host->set_prefix(val);
}

void team_config::set_ip_or_host_suffix(const std::string &val) {
  m_ip_or_host->set_suffix(val);
}

std::string team_config::ip_or_host() const {
  return m_ip_or_host->value();
}

std::string team_config::ip_subnet() const {
  return m_ip_or_host->ip_v4_subnet();
}

bool team_config::is_active() const {
  return m_active->value();
}

std::string team_config::logo_path() const {
  return m_logo->value();
}
std::string team_config::logo_big_path() const {
  return m_logo_big->value();
}

long team_config::updated_time() {
  return m_updated_time;
}

nlohmann::json team_config::to_json() {
  nlohmann::json teamInfo;
  teamInfo[json_fields::ID] = id();
  teamInfo[json_fields::NAME] = name();
  teamInfo[json_fields::DESCRIPTION] = description();
  teamInfo[json_fields::IP_OR_HOST] = ip_or_host();
  teamInfo[json_fields::LOGO] = "./logo/team/" + id();
  teamInfo[json_fields::LOGO_BIG] = "./logo/big/team/" + id();
  teamInfo[json_fields::UPDATED] = updated_time();
  return teamInfo;
}

} // namespace ctf01d
