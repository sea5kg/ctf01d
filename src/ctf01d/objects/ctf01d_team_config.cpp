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
#include "ctf01d/utils/ctf01d_logger.h"
#include <wsjcpp_core.h>
#include <vector>
#include <algorithm>

namespace ctf01d {

team_config::team_config() {
  // normal, red, blue, guest, inactive, disqualified
  m_id = ctf01d::var_string::create({"id"}, "normal", m_vars);
  m_type = ctf01d::var_string::create({"type"}, "normal", m_vars); // TODO var_types
  m_name = ctf01d::var_string::create({"name"}, "normal", m_vars);
  m_active = ctf01d::var_bool::create({"active"}, true, m_vars);
  m_logo = ctf01d::var_file::create({"logo"}, "", "", m_vars);
  m_logo_big = ctf01d::var_file::create({"logo-big"}, "", "", m_vars);
  m_ip_or_host = ctf01d::var_ip_or_host::create({"ip-or-host"}, m_vars); // TODO var_ip_or_host
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
    "normal",
    "red",
    "blue",
    "guest",
    "disqualified",
  };
  if (std::find(allowed_types.begin(), allowed_types.end(), m_type->value()) == allowed_types.end()) {
    err = "Didn't allowed team.type: '" + m_type->value() + "'";
    ctf01d::log::err(TAG, err);
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

int team_config::get_logo_last_modified_time() {
  return m_logo_last_modified_time;
}

} // namespace ctf01d
