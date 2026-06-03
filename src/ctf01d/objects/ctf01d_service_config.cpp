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

#include "ctf01d_service_config.h"
#include <wsjcpp_core.h>

namespace ctf01d {

service_config::service_config() {
  TAG = "ctf01d::service_config";

  m_id = ctf01d::var_string::create({"id"}, "", m_vars);
  m_name = ctf01d::var_string::create({"name"}, "", m_vars);
  m_enabled = ctf01d::var_bool::create({"enabled"}, true, m_vars);
  m_logo = ctf01d::var_file::create({"logo"}, "", "", m_vars);
  m_logo_big = ctf01d::var_file::create({"logo-big"}, "", "", m_vars);
  m_script_path = ctf01d::var_string::create({"script-relative-path"}, "", m_vars);
  m_script_timeout_in_seconds = ctf01d::var_int::create({"script-timeout-in-seconds"}, 5, m_vars);
  m_script_timeout_in_seconds->set_minimum(1);
  m_round_in_seconds = ctf01d::var_int::create({"round-in-seconds"}, 15, m_vars);
  m_round_in_seconds->set_minimum(1);
  // m_round_in_seconds->set_maximum(1);

  // not in the scope
  m_script_dir = std::make_shared<ctf01d::var_dir>(std::vector<std::string>({"script_dir"}), "", "");
}

bool service_config::read(WsjcppYamlCursor &cursor, const std::string &work_dir, std::string &err) {
  m_work_dir = work_dir;
  m_logo->set_root_dir(m_work_dir);
  m_logo_big->set_root_dir(m_work_dir);
  if (!m_vars.read(cursor, err)) {
    return false;
  }
  if (m_enabled->value()) {
    m_script_dir->set_root_dir(m_work_dir);
    if (!m_script_dir->set_value("checker_" + m_id->value(), err)) {
      return false;
    }
  }

  if (m_round_in_seconds->value() < m_script_timeout_in_seconds->value()*3) {
    err = "";
    return false;
  }
  return true;
}

std::string service_config::id() const {
  return m_id->value();
}

std::string service_config::name() const {
  return m_name->value();
}

std::string service_config::script_path() const {
  return m_script_path->value();
}

std::string service_config::script_dir() const {
  return m_script_dir->value();
}

bool service_config::is_enabled() const {
  return m_enabled->value();
}

int service_config::script_timeout_in_seconds() const {
  return m_script_timeout_in_seconds->value();
}

int service_config::round_in_seconds() const {
  return m_round_in_seconds->value();
}

std::string service_config::logo_path() const {
  return m_logo->value();
}

std::string service_config::logo_big_path() const {
  return m_logo_big->value();
}


} // namespace ctf01d
