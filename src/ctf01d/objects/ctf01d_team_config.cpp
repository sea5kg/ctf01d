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

namespace ctf01d {

team_config::team_config() {
  // normal, red, blue, guest, inactive, disqualified
  m_type = ctf01d::var_string::create({"type"}, "normal", m_vars);
  m_logo = ctf01d::var_file::create({"logo"}, "", "", m_vars);
  m_big_logo = ctf01d::var_file::create({"big-logo"}, "", "", m_vars);
}

bool team_config::read(WsjcppYamlCursor &cursor, const std::string &work_dir, std::string &err) {
  m_work_dir = work_dir;
  m_logo->set_root_dir(m_work_dir);
  m_big_logo->set_root_dir(m_work_dir);
  if (!m_vars.read(cursor, err)) {
    return false;
  }
  // if (m_enabled->value()) {
  //   m_script_dir->set_root_dir(m_work_dir);
  //   if (!m_script_dir->set_value("checker_" + m_id->value(), err)) {
  //     return false;
  //   }
  // }

  return true;
}

void team_config::setId(const std::string &sTeamId){
  m_sTeamID = sTeamId;
}

const std::string &team_config::getId() const {
  return m_sTeamID;
}

void team_config::setName(const std::string &sName){
  m_sName = sName;
}

const std::string &team_config::getName() const {
  return m_sName;
}

void team_config::setIpAddress(const std::string &sIpAddress){
  m_sIpAddress = sIpAddress;
}

const std::string &team_config::ipAddress() const {
  return m_sIpAddress;
}

void team_config::setActive(bool bActive){
  m_bActive = bActive;
}

bool team_config::isActive() const {
  return m_bActive;
}

void team_config::setLogo(const std::string &sLogo){
  m_sLogo = sLogo;
}

const std::string &team_config::logo() const {
  return m_sLogo;
}

int team_config::getLogoLastWriteTime() {
  return m_nLogoLastWriteTime;
}

} // namespace ctf01d
