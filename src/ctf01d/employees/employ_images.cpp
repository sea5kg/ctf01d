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

#include "employ_images.h"
#include <wsjcpp_core.h>
#include <filesystem>
#include "ctf01d/employees/employ_config.h"

REGISTRY_WSJCPP_EMPLOY(EmployImages)

EmployImages::EmployImages()
: WsjcppEmployBase({ EmployImages::name() }, { EmployConfig::name() }) {
  TAG = EmployImages::name();
  m_nLastUpdateChangeTimeLogosInSec = WsjcppCore::getCurrentTimeInSeconds();
}

bool EmployImages::init(const std::string &sName, bool bSilent) {
  WsjcppLog::info(TAG, "init");
  return true;
}

bool EmployImages::deinit(const std::string &sName, bool bSilent) {
  WsjcppLog::info(TAG, "deinit");
  return true;
}

bool EmployImages::load_team_logo(const std::string &team_id, const std::string &filepath) {
  return load_logo("team/" + team_id, filepath);
}

bool EmployImages::load_team_big_logo(const std::string &team_id, const std::string &filepath) {
  return load_logo("big/team/" + team_id, filepath);
}

bool EmployImages::load_service_logo(const std::string &service_id, const std::string &filepath) {
  return load_logo("service/" + service_id, filepath);
}

bool EmployImages::load_service_big_logo(const std::string &service_id, const std::string &filepath) {
  return load_logo("big/service/" + service_id, filepath);
}

std::shared_ptr<ctf01d::image> EmployImages::find_image(const std::string &id) {
  std::map<std::string, std::shared_ptr<ctf01d::image>>::iterator it = m_images.find(id);
  if (it != m_images.end()) {
    return it->second;
  }
  return nullptr;
}

bool EmployImages::update_last_change_time() {
  if (WsjcppCore::getCurrentTimeInSeconds() - m_nLastUpdateChangeTimeLogosInSec < 30) {
    return false;
  }
  m_nLastUpdateChangeTimeLogosInSec = WsjcppCore::getCurrentTimeInSeconds();
  WsjcppLog::info(TAG, "updateLastWriteTime for team's logos");
  bool bHasChanges = false;
  {
    std::map<std::string, std::shared_ptr<ctf01d::image>>::iterator it = m_images.begin();
    while (it != m_images.end()) {
      auto img = it->second;
      std::filesystem::file_time_type ftime = std::filesystem::last_write_time(img->filepath().c_str());
      long last_changed_time = std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count();
      if (last_changed_time != img->last_modified_time()) {
        bHasChanges = true;
        delete img->pBuffer;
        img->pBuffer = nullptr;
        img->nBufferSize = 0;
        WsjcppCore::readFileToBuffer(img->filepath(), &(img->pBuffer), img->nBufferSize);
        // img->nLastWriteTime = last_changed_time;
      }
      it++;
    }
  }
  return bHasChanges;
}

void EmployImages::update_scoreboard_json(nlohmann::json &jsonScoreboard) {
  // TODO
  // {
  //   std::map<std::string, std::shared_ptr<ctf01d::image>>::iterator it = m_teams_logo.begin();
  //   while (it != m_teams_logo.end()) {
  //     auto img = it->second;
  //     jsonScoreboard["scoreboard"][img->id()]["logo_last_updated"] = img->nLastWriteTime;
  //     it++;
  //   }
  // }
  // {
  //   std::map<std::string, std::shared_ptr<ctf01d::image>>::iterator it = m_teams_big_logo.begin();
  //   while (it != m_teams_big_logo.end()) {
  //     auto img = it->second;
  //     jsonScoreboard["scoreboard"][img->id()]["logo_last_updated"] = img->nLastWriteTime;
  //     it++;
  //   }
  // }
}

bool EmployImages::load_logo(const std::string &id, const std::string &filepath) {
   if (!WsjcppCore::fileExists(filepath)) {
    WsjcppLog::err(TAG, "File '" + filepath + "' did not found");
    return false;
  }
  std::shared_ptr<ctf01d::image> img = std::make_shared<ctf01d::image>(id);
  if (!img->reload_from_file(filepath)) {
    WsjcppLog::throw_err(TAG, "Could not read file '" + filepath + "'");
    return false;
  }
  m_images[id] = img;
  WsjcppLog::info(TAG, "Loaded image " + filepath + " for " + id + " (last write time file: " + std::to_string(img->last_modified_time()) + ")");
  return true;
}