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

#include <wsjcpp_employees.h>
#include <json.hpp>
#include "ctf01d/objects/ctf01d_team_logo.h"

class EmployTeamLogos : public WsjcppEmployBase {
public:
  EmployTeamLogos();
  static std::string name() { return "EmployTeamLogos"; }
  virtual bool init(const std::string &name, bool silent) override;
  virtual bool deinit(const std::string &name, bool silent) override;
  bool load_team_logo(const std::string &team_id, const std::string &filepath);
  bool load_team_big_logo(const std::string &team_id, const std::string &filepath);
  bool load_service_logo(const std::string &service_id, const std::string &filepath);
  bool load_service_big_logo(const std::string &service_id, const std::string &filepath);
  Ctf01dTeamLogo *find_logo_team(const std::string &team_id);
  Ctf01dTeamLogo *find_logo_big_team(const std::string &team_id);
  Ctf01dTeamLogo *find_logo_service(const std::string &team_id);
  Ctf01dTeamLogo *find_logo_big_service(const std::string &team_id);
  bool update_last_change_time();
  void update_scoreboard_json(nlohmann::json &jsonScoreboard);

private:
  std::string TAG;
  bool load_logo(const std::string &team_id, const std::string &filepath, std::map<std::string, Ctf01dTeamLogo *> &logos);

  std::map<std::string, Ctf01dTeamLogo *> m_teams_logo;
  std::map<std::string, Ctf01dTeamLogo *> m_teams_big_logo;
  std::map<std::string, Ctf01dTeamLogo *> m_services_logo;
  std::map<std::string, Ctf01dTeamLogo *> m_services_big_logo;
  int m_nLastUpdateChangeTimeLogosInSec;
};
