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

#include "employ_team_logos.h"
#include <wsjcpp_core.h>
#include <filesystem>
#include "ctf01d/employees/employ_config.h"

REGISTRY_WSJCPP_EMPLOY(EmployTeamLogos)

EmployTeamLogos::EmployTeamLogos()
: WsjcppEmployBase({ EmployTeamLogos::name() }, { EmployConfig::name() }) {
  TAG = EmployTeamLogos::name();
  m_nLastUpdateChangeTimeLogosInSec = WsjcppCore::getCurrentTimeInSeconds();
}

bool EmployTeamLogos::init(const std::string &sName, bool bSilent) {
  WsjcppLog::info(TAG, "init");
  return true;
}

bool EmployTeamLogos::deinit(const std::string &sName, bool bSilent) {
  WsjcppLog::info(TAG, "deinit");
  return true;
}

bool EmployTeamLogos::load_team_logo(const std::string &team_id, const std::string &filepath) {
  if (!load_logo(team_id, filepath, m_teams_logo)) {
    return false;
  }
  WsjcppLog::info(TAG, "Loaded logo: " + filepath + " for team " + team_id + " (last write time file: " + std::to_string(m_teams_logo[team_id]->nLastWriteTime) + ")");
  return true;
}

bool EmployTeamLogos::load_team_big_logo(const std::string &team_id, const std::string &filepath) {
  if (!load_logo(team_id, filepath, m_teams_big_logo)) {
    return false;
  }
  WsjcppLog::info(TAG, "Loaded big-logo: " + filepath + " for team " + team_id + " (last write time file: " + std::to_string(m_teams_big_logo[team_id]->nLastWriteTime) + ")");
  return true;
}

bool EmployTeamLogos::load_service_logo(const std::string &service_id, const std::string &filepath) {
  if (!load_logo(service_id, filepath, m_services_logo)) {
    return false;
  }
  WsjcppLog::info(TAG, "Loaded logo: " + filepath + " for service " + service_id + " (last write time file: " + std::to_string(m_services_logo[service_id]->nLastWriteTime) + ")");
  return true;
}

bool EmployTeamLogos::load_service_big_logo(const std::string &service_id, const std::string &filepath) {
  if (!load_logo(service_id, filepath, m_services_big_logo)) {
    return false;
  }
  WsjcppLog::info(TAG, "Loaded big-logo: " + filepath + " for service " + service_id + " (last write time file: " + std::to_string(m_services_big_logo[service_id]->nLastWriteTime) + ")");
  return true;
}

Ctf01dTeamLogo *EmployTeamLogos::find_logo_team(const std::string &team_id) {
  std::map<std::string, Ctf01dTeamLogo *>::iterator it = m_teams_logo.find(team_id);
  if (it != m_teams_logo.end()) {
    return it->second;
  }
  return nullptr;
}

Ctf01dTeamLogo *EmployTeamLogos::find_logo_big_team(const std::string &team_id) {
  std::map<std::string, Ctf01dTeamLogo *>::iterator it = m_teams_big_logo.find(team_id);
  if (it != m_teams_big_logo.end()) {
    return it->second;
  }
  return nullptr;
}

Ctf01dTeamLogo *EmployTeamLogos::find_logo_service(const std::string &service_id) {
  std::map<std::string, Ctf01dTeamLogo *>::iterator it = m_services_logo.find(service_id);
  if (it != m_services_logo.end()) {
    return it->second;
  }
  return nullptr;
}

Ctf01dTeamLogo *EmployTeamLogos::find_logo_big_service(const std::string &service_id) {
  std::map<std::string, Ctf01dTeamLogo *>::iterator it = m_services_big_logo.find(service_id);
  if (it != m_services_big_logo.end()) {
    return it->second;
  }
  return nullptr;
}

bool EmployTeamLogos::update_last_change_time() {
  if (WsjcppCore::getCurrentTimeInSeconds() - m_nLastUpdateChangeTimeLogosInSec < 30) {
    return false;
  }
  m_nLastUpdateChangeTimeLogosInSec = WsjcppCore::getCurrentTimeInSeconds();
  WsjcppLog::info(TAG, "updateLastWriteTime for team's logos");
  bool bHasChanges = false;
  std::map<std::string, Ctf01dTeamLogo *>::iterator it = m_teams_logo.begin();
  while (it != m_teams_logo.end()) {
    Ctf01dTeamLogo *pTeamLogo = it->second;
    std::filesystem::file_time_type ftime = std::filesystem::last_write_time(pTeamLogo->sFilepath.c_str());
    long last_changed_time = std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count();
    if (last_changed_time != pTeamLogo->nLastWriteTime) {
      bHasChanges = true;
      delete pTeamLogo->pBuffer;
      pTeamLogo->pBuffer = nullptr;
      pTeamLogo->nBufferSize = 0;
      WsjcppCore::readFileToBuffer(pTeamLogo->sFilepath, &(pTeamLogo->pBuffer), pTeamLogo->nBufferSize);
      pTeamLogo->nLastWriteTime = last_changed_time;
    }
    it++;
  }
  std::map<std::string, Ctf01dTeamLogo *>::iterator it2 = m_teams_big_logo.begin();
  while (it2 != m_teams_big_logo.end()) {
    Ctf01dTeamLogo *pTeamLogo = it2->second;
    std::filesystem::file_time_type ftime = std::filesystem::last_write_time(pTeamLogo->sFilepath.c_str());
    long last_changed_time = std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count();
    if (last_changed_time != pTeamLogo->nLastWriteTime) {
      bHasChanges = true;
      delete pTeamLogo->pBuffer;
      pTeamLogo->pBuffer = nullptr;
      pTeamLogo->nBufferSize = 0;
      WsjcppCore::readFileToBuffer(pTeamLogo->sFilepath, &(pTeamLogo->pBuffer), pTeamLogo->nBufferSize);
      pTeamLogo->nLastWriteTime = last_changed_time;
    }
    it2++;
  }
  return bHasChanges;
}

void EmployTeamLogos::update_scoreboard_json(nlohmann::json &jsonScoreboard) {
  std::map<std::string, Ctf01dTeamLogo *>::iterator it = m_teams_logo.begin();
  while (it != m_teams_logo.end()) {
    Ctf01dTeamLogo *pTeamLogo = it->second;
    jsonScoreboard["scoreboard"][pTeamLogo->sTeamId]["logo_last_updated"] = pTeamLogo->nLastWriteTime;
    it++;
  }
  std::map<std::string, Ctf01dTeamLogo *>::iterator it2 = m_teams_big_logo.begin();
  while (it2 != m_teams_big_logo.end()) {
    Ctf01dTeamLogo *pTeamLogo = it2->second;
    jsonScoreboard["scoreboard"][pTeamLogo->sTeamId]["logo_last_updated"] = pTeamLogo->nLastWriteTime;
    it2++;
  }
}

bool EmployTeamLogos::load_logo(const std::string &id, const std::string &filepath, std::map<std::string, Ctf01dTeamLogo *> &logos) {
   if (!WsjcppCore::fileExists(filepath)) {
    WsjcppLog::err(TAG, "File '" + filepath + "' did not found");
    return false;
  }
  Ctf01dTeamLogo *pLogo = new Ctf01dTeamLogo();
  pLogo->sTeamId = id;
  pLogo->pBuffer = nullptr;
  pLogo->nBufferSize = 0;
  pLogo->sFilename = WsjcppCore::extractFilename(filepath);
  pLogo->sFilepath = filepath;
  if (!WsjcppCore::readFileToBuffer(filepath, &(pLogo->pBuffer), (pLogo->nBufferSize))) {
    delete pLogo;
    WsjcppLog::throw_err(TAG, "Could not read file '" + filepath + "'");
    return false;
  }
  std::filesystem::file_time_type ftime = std::filesystem::last_write_time(filepath.c_str());
  pLogo->nLastWriteTime = std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count();
  // add only if file found
  logos[id] = pLogo;
  return true;
}