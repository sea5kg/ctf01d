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
#include "ctf01d/objects/ctf01d_scoreboard.h"
#include "ctf01d/objects/ctf01d_service_def.h"
#include "ctf01d/objects/ctf01d_team_def.h"

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
  std::vector<Ctf01dServiceDef> &servicesConf();

  // teams configuration
  std::vector<Ctf01dTeamDef> &teamsConf();

  // scoreboard configuration
  int scoreboardPort() const ;
  std::string scoreboardHtmlFolder() const;
  bool scoreboardRandom() const;

  // game configuration
  std::string gameId() const;
  std::string gameName() const;
  int flagTimeliveInMin() const;
  int getBasicCostsStolenFlagInPoints() const;
  int getCostDefenseFlagInPoints10() const;
  int gameStartUTCInSec() const;
  int gameEndUTCInSec() const;

  bool gameHasCoffeeBreak();
  int gameCoffeeBreakStartUTCInSec();
  int gameCoffeeBreakEndUTCInSec();

  std::shared_ptr<Ctf01dScoreboard> scoreboard();

  void doExtractFilesIfNotExists();

private:
  bool applyGameConf(WsjcppYaml &yamlConfig);
  bool applyScoreboardConf(WsjcppYaml &yamlConfig);
  bool applyCheckersConf(WsjcppYaml &yamlConfig);
  bool readTeamsConf(WsjcppYaml &yamlConfig);
  void tryLoadFromEnv(const std::string &sEnvName, std::string &sValue, const std::string &sDescription);
  bool isValidIPv4(const std::string &sValue, std::string &sError);

  std::string TAG;
  std::string m_sWorkDir;
  bool m_bAppliedConfig;

  std::shared_ptr<Ctf01dScoreboard> m_pScoreboard;
  int m_nScoreboardPort;
  std::string m_sScoreboardHtmlFolder;
  bool m_bScoreboardRandom;

  // game conf
  int m_nFlagTimeliveInMin;
  int m_nBasicCostsStolenFlagInPoints;
  int m_nCostDefenseFlagInPoints10;
  std::string m_sGameId;
  std::string m_sGameName;
  std::string m_sGameStart;
  std::string m_sGameEnd;
  int m_nGameStartUTCInSec; // UTC in seconds
  int m_nGameEndUTCInSec; // UTC in seconds

  bool m_bHasCoffeeBreak;
  std::string m_sGameCoffeeBreakStart;
  std::string m_sGameCoffeeBreakEnd;
  int m_nGameCoffeeBreakStartUTCInSec; // UTC in seconds
  int m_nGameCoffeeBreakEndUTCInSec; // UTC in seconds

  std::vector<Ctf01dTeamDef> m_vTeamsConf;
  std::vector<Ctf01dServiceDef> m_vServicesConf;
};
