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
#include <wsjcpp_yaml.h>
#include "ctf01d/objects/ctf01d_var.h"
#include "ctf01d/objects/ctf01d_scoreboard.h"
#include "ctf01d/objects/ctf01d_service_config.h"
#include "ctf01d/objects/ctf01d_team_def.h"
#include "ctf01d/objects/ctf01d_files_watcher.h"

class EmployConfig : public WsjcppEmployBase {
public:
  EmployConfig();
  ~EmployConfig();
  static std::string name() { return "EmployConfig"; }
  virtual bool init(const std::string &sName, bool bSilent) override;
  virtual bool deinit(const std::string &sName, bool bSilent) override;
  void setWorkDir(const std::string &sWorkDir);
  std::string getWorkDir();

  bool applyConfig();

  // services configuration
  std::vector<ctf01d::service_config> &servicesConf();

  // teams configuration
  std::vector<Ctf01dTeamDef> &teamsConf();

  // scoreboard configuration
  int scoreboardPort() const ;
  std::string scoreboardHtmlFolder() const;
  bool scoreboardRandom() const;

  // game configuration
  std::string gameId() const;
  std::string gameName() const;
  int flagLifetimeInSeconds() const;
  std::shared_ptr<ctf01d::var_int> get_flag_cost_in_points() const;
  int gameStartUTCInSec() const;
  int gameEndUTCInSec() const;

  bool gameHasCoffeeBreak();
  int gameCoffeeBreakStartUTCInSec();
  int gameCoffeeBreakEndUTCInSec();

  std::shared_ptr<Ctf01dScoreboard> scoreboard();

  void doExtractFilesIfNotExists();

private:
  bool checkYamlMainKeys(WsjcppYaml &yamlConfig);
  bool applyGameConf(WsjcppYaml &yamlConfig);
  bool applyServicesConfig(WsjcppYaml &yamlConfig);
  bool readTeamsConf(WsjcppYaml &yamlConfig);
  bool isValidIPv4(const std::string &sValue, std::string &sError);
  bool initWorkDir();
  bool initLogger();

  std::string TAG;
  std::string m_sWorkDir;
  std::string m_sConfigFilepath;
  bool m_bAppliedConfig;

  // scoreboard config
  std::shared_ptr<Ctf01dScoreboard> m_pScoreboard;
  std::shared_ptr<ctf01d::var_int> m_scoreboard_port;
  std::shared_ptr<ctf01d::var_dir> m_scoreboard_html_folder;
  std::shared_ptr<ctf01d::var_bool> m_scoreboard_random;

  std::vector<std::shared_ptr<ctf01d::var>> m_vars;

  // game config
  std::shared_ptr<ctf01d::var_int> m_flag_lifetime_in_seconds;
  std::shared_ptr<ctf01d::var_int> m_flag_cost_in_points;
  std::shared_ptr<ctf01d::var_string> m_game_id;
  std::shared_ptr<ctf01d::var_string> m_game_name;
  std::shared_ptr<ctf01d::var_datetime> m_game_start_utc;
  std::shared_ptr<ctf01d::var_datetime> m_game_end_utc;
  bool m_bHasCoffeeBreak;
  std::shared_ptr<ctf01d::var_datetime> m_game_coffee_break_start_utc;
  std::shared_ptr<ctf01d::var_datetime> m_game_coffee_break_end_utc;

  // teams config
  std::vector<Ctf01dTeamDef> m_vTeamsConf;

  // services config
  std::vector<ctf01d::service_config> m_vServicesConf;

  // hot-reload: for reload config in runtime
  std::shared_ptr<Ctf01dFilesWatcher> m_files_watcher;
};
