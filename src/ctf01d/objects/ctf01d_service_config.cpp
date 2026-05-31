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
    m_nScriptWaitInSec = 10;
    m_bEnabled = true;
    m_round_in_seconds = ctf01d::var_int::create({"round_in_seconds"}, 15);
    m_round_in_seconds->set_minimum(1);
    m_vars.push_back(m_round_in_seconds);
    // m_round_in_seconds->set_maximum(1);
}

bool service_config::read(WsjcppYamlCursor &cursor, std::string &err) {
  bool var_errors = false;
  for (int i = 0; i < m_vars.size(); ++i) {
    std::shared_ptr<ctf01d::var> var = m_vars[i];
    std::string err;
    if (!var->read(cursor, err)) {
      WsjcppLog::err(TAG, err);
      var_errors = true;
      continue;
    }
    WsjcppLog::info(TAG, var->name() + ": " + var->to_string());
  }
  if (var_errors) {
    return false;
  }
  if (m_round_in_seconds->value() < m_nScriptWaitInSec*3) {
    err = "";
    return false;
  }
  return true;
}

void service_config::setId(const std::string &sServiceID){
    m_sID = sServiceID;
}

const std::string &service_config::id() const {
    return m_sID;
}

void service_config::setName(const std::string &sName){
    m_sName = sName;
}

const std::string &service_config::name() const {
    return m_sName;
}

void service_config::setScriptPath(const std::string &sScriptPath){
    m_sScriptPath = sScriptPath;
}

const std::string &service_config::scriptPath() const {
    return m_sScriptPath;
}

void service_config::setScriptDir(const std::string &sScriptDir) {
    m_sScriptDir = sScriptDir;
}

const std::string &service_config::scriptDir() const {
    return m_sScriptDir;
}

void service_config::setEnabled(bool bEnabled){
    m_bEnabled = bEnabled;
}

bool service_config::isEnabled() const {
    return m_bEnabled;
}

void service_config::setScriptWaitInSec(int nSec){
    m_nScriptWaitInSec = nSec;
    if(m_nScriptWaitInSec < 1){
        m_nScriptWaitInSec = 10;
    }
}

int service_config::scriptWaitInSec() const {
    return m_nScriptWaitInSec;
}

int service_config::round_in_seconds() const {
    return m_round_in_seconds->value();
}

} // namespace ctf01d
