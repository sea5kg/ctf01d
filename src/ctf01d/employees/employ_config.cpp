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

#include "employ_config.h"
#include <wsjcpp_core.h>
#include <sstream>
#include <ctime>
#include <locale>
#include <date.h>
#include <iostream>
#include <sstream>
#include <wsjcpp_core.h>
#include <wsjcpp_yaml.h>
#include "ctf01d/employees/employ_team_logos.h"
#include "ctf01d/include/ctf01d_globals.h"

#include <sys/stat.h>
#include <stdio.h>

REGISTRY_WSJCPP_EMPLOY(EmployConfig)

EmployConfig::EmployConfig()
: WsjcppEmployBase({ EmployConfig::name() }, {}) {
  TAG = EmployConfig::name();
  m_files_watcher = std::make_shared<Ctf01dFilesWatcher>();

  // game options
  m_game_id = ctf01d::var_string::create({"game", "id"}, "test", m_vars);
  m_game_name = ctf01d::var_string::create({"game", "name"}, "Test", m_vars);

  m_bAppliedConfig = false;
  m_flag_lifetime_in_seconds = ctf01d::var_int::create({"game", "flag_lifetime_in_seconds"}, 60, m_vars);
  m_flag_lifetime_in_seconds->set_minimum(1);
  m_flag_lifetime_in_seconds->set_maximum(MAX_FLAG_LIFETIME_SECONDS);

  m_flag_cost_in_points = ctf01d::var_int::create({"game", "flag_cost_in_points"}, 100, m_vars);
  m_flag_cost_in_points->set_minimum(1);
  m_flag_cost_in_points->set_maximum(MAX_FLAG_COST_IN_POINTS);

  m_game_start_utc = ctf01d::var_datetime::create({"game", "start_utc"}, "2023-11-12 16:00:00", m_vars);
  m_game_end_utc = ctf01d::var_datetime::create({"game", "end_utc"}, "2030-11-12 22:00:00", m_vars);
  m_game_coffee_break_start_utc = ctf01d::var_datetime::create({"game", "coffee_break_start"}, "2023-11-12 20:00:00", m_vars);
  m_game_coffee_break_end_utc = ctf01d::var_datetime::create({"game", "coffee_break_end"}, "2023-11-12 21:00:00", m_vars);

  m_bHasCoffeeBreak = false;

  // scoreboard config

  m_scoreboard_port = ctf01d::var_int::create({"scoreboard", "port"}, 8080, m_vars);
  m_scoreboard_port->set_minimum(MIN_TCP_PORT);
  m_scoreboard_port->set_maximum(MAX_TCP_PORT);

  m_scoreboard_random = ctf01d::var_bool::create({"scoreboard", "random"}, false, m_vars);
  m_scoreboard_html_folder = ctf01d::var_dir::create({"scoreboard", "htmlfolder"}, "./html", m_sWorkDir, m_vars);

  m_pScoreboard = nullptr;
}

EmployConfig::~EmployConfig() {
  // TODO cleanup
  m_vars.clear();
}

bool EmployConfig::init(const std::string &sName, bool bSilent) {
  if (!initWorkDir()) {
    return false;
  }

  if (!initLogger()) {
    return false;
  }

  this->doExtractFilesIfNotExists();

  m_sConfigFilepath = m_sWorkDir + "/config.yml";
  m_files_watcher->watchFile(m_sConfigFilepath);

  if (!this->applyConfig()) {
    WsjcppLog::err(TAG, "Configuration file has some problems");
    return false;
  }

  return true;
}

bool EmployConfig::deinit(const std::string &sName, bool bSilent) {
    WsjcppLog::info(TAG, "deinit");
    return true;
}

void EmployConfig::setWorkDir(const std::string &sWorkDir) {
  if (m_sWorkDir != "" && m_sWorkDir != sWorkDir) {
    std::cout << "Changed work-dir to '" + sWorkDir + "'" << std::endl;
  }
  m_sWorkDir = sWorkDir;
  m_sConfigFilepath = m_sWorkDir + "/config.yml";
  m_scoreboard_html_folder->set_root_dir(m_sWorkDir);
}

std::string EmployConfig::getWorkDir() {
    return m_sWorkDir;
}

bool EmployConfig::applyConfig() {
  if (m_bAppliedConfig) {
    return true;
  }

  m_bAppliedConfig = false;
  WsjcppLog::info(TAG, "Loading configuration...");

  std::string sConfigFile = m_sWorkDir + "/config.yml";
  WsjcppLog::info(TAG, "Reading config: " + sConfigFile);

  if (!WsjcppCore::fileExists(sConfigFile)) {
    WsjcppLog::err(TAG, "File " + sConfigFile + " does not exists");
    return false;
  }

  WsjcppYaml yamlConfig;
  std::string sError;
  if (!yamlConfig.loadFromFile(sConfigFile, sError)) {
    WsjcppLog::err(TAG, "Could not parse " + sConfigFile + ", reason: " + sError);
    return false;
  }

  if (!checkYamlMainKeys(yamlConfig)) {
    return false;
  }

  bool var_errors = false;
  for (int i = 0; i < m_vars.size(); ++i) {
    std::shared_ptr<ctf01d::var> var = m_vars[i];
    std::string err;
    auto cursor = yamlConfig.getCursor();
    if (!var->read(cursor, err)) {
      var_errors = true;
      continue;
    }
    WsjcppLog::info(TAG, var->name() + ": " + var->to_string());
  }
  if (var_errors) {
    return false;
  }

  // apply the game config
  if (!this->applyGameConf(yamlConfig)) {
    return false;
  }

  if (!this->applyServicesConfig(yamlConfig)) {
    return false;
  }

  if (!this->readTeamsConf(yamlConfig)) {
    return false;
  }

  // scoreboard
  m_pScoreboard = std::make_shared<Ctf01dScoreboard>(
    m_scoreboard_random->value(),
    m_game_start_utc->value_in_seconds(),
    m_game_end_utc->value_in_seconds(),
    m_game_coffee_break_start_utc->value_in_seconds(),
    m_game_coffee_break_end_utc->value_in_seconds()
  );

  m_bAppliedConfig = true;
  return m_bAppliedConfig;
}

std::vector<Ctf01dTeamDef> &EmployConfig::teamsConf() {
  return m_vTeamsConf;
}

std::vector<ctf01d::service_config> &EmployConfig::servicesConf() {
  return m_vServicesConf;
}

int EmployConfig::scoreboardPort() const {
  return m_scoreboard_port->value();
}

std::string EmployConfig::scoreboardHtmlFolder() const {
  return m_scoreboard_html_folder->value();
}

bool EmployConfig::scoreboardRandom() const {
  return m_scoreboard_random->value();
}

std::string EmployConfig::gameId() const {
  return m_game_id->value();
}

std::string EmployConfig::gameName() const  {
  return m_game_name->value();
}

int EmployConfig::flagLifetimeInSeconds() const  {
  return m_flag_lifetime_in_seconds->value();
}

std::shared_ptr<ctf01d::var_int> EmployConfig::get_flag_cost_in_points() const {
  return m_flag_cost_in_points;
}

int EmployConfig::gameStartUTCInSec() const {
  // TODO return var
  return m_game_start_utc->value_in_seconds();
}

int EmployConfig::gameEndUTCInSec() const {
  // TODO return var
  return m_game_end_utc->value_in_seconds();
}

bool EmployConfig::gameHasCoffeeBreak() {
  return m_bHasCoffeeBreak;
}

int EmployConfig::gameCoffeeBreakStartUTCInSec() {
  return m_game_coffee_break_start_utc->value_in_seconds();
}

int EmployConfig::gameCoffeeBreakEndUTCInSec() {
  return m_game_coffee_break_end_utc->value_in_seconds();
}

std::shared_ptr<Ctf01dScoreboard> EmployConfig::scoreboard() {
  return m_pScoreboard;
}

void EmployConfig::doExtractFilesIfNotExists() {
  std::string sError;
  if (!WsjcppCore::dirExists(m_sWorkDir + "/logs")) {
    WsjcppCore::makeDir(m_sWorkDir + "/logs");
    if (!WsjcppCore::setFilePermissions(m_sWorkDir + "/logs", WsjcppFilePermissions(0x776), sError)) {
      WsjcppLog::throw_err(TAG, sError);
    }
  }

  if (!WsjcppCore::fileExists(m_sWorkDir + "/config.yml")) {
    WsjcppLog::warn(TAG, "Extracting config.yml and files");
    WsjcppLog::warn(TAG, "Extracting checker_example_*");
    const std::vector<WsjcppResourceFile*> &vFiles = WsjcppResourcesManager::list();
    std::vector<std::string> vExecutableFiles;
    for (int i = 0; i < vFiles.size(); i++) {
      std::string sFilepath = vFiles[i]->getFilename();
      if (sFilepath.rfind("./data_sample/checker_example_", 0) == 0) {
        std::vector<std::string> vPath = WsjcppCore::split(sFilepath, "/");
        std::string sDirname = vPath[2];
        vPath.erase (vPath.begin(),vPath.begin()+3);
        std::string sNewFilepath = WsjcppCore::join(vPath, "/");
        sNewFilepath = wsjcpp::normalizeFilePath(m_sWorkDir + "/" + sDirname + "/" + sNewFilepath);
        if (!WsjcppCore::fileExists(sNewFilepath)) {
          std::cout << "Extracting file '" << sFilepath << "' to '" << sNewFilepath << "'" << std::endl;
        } else {
          std::cout << "File '" << sNewFilepath << "' already exists. Skip." << std::endl;
          continue;
        }

        // prepare folder
        std::string sFolder = wsjcpp::normalizeFilePath(m_sWorkDir + "/" + sDirname + "/");
        if (!WsjcppCore::dirExists(sFolder)) {
          WsjcppCore::makeDir(sFolder);
        }

        if (!WsjcppCore::writeFile(sNewFilepath, vFiles[i]->getBuffer(), vFiles[i]->getBufferSize())) {
          std::cout << "ERROR. Could not write file. " << std::endl;
          continue;
        } else {
          std::cout << "Successfully created file. " << std::endl;
          // TODO redesign set permission via wsjcpp
          if (chmod(sNewFilepath.c_str(), S_IRWXU|S_IRWXG) != 0) {
            std::cout << "ERROR. Could not change permissions for. " << sNewFilepath << std::endl;
          } else {
            struct stat info;
            stat(sNewFilepath.c_str(), &info);
            printf("after chmod(), permissions are: %08x\n", info.st_mode);
          }
        }
      }
    }

    WsjcppResourceFile* pConfigYml = WsjcppResourcesManager::get("./data_sample/config.yml");
    std::string sNewFilepath = wsjcpp::normalizeFilePath(m_sWorkDir + "/config.yml");
    if (!WsjcppCore::writeFile(sNewFilepath, pConfigYml->getBuffer(), pConfigYml->getBufferSize())) {
      std::cout << "ERROR. Could not write file. " << std::endl;
    } else {
      std::cout << "Successfully created file. " << std::endl;
    }
  }

  if (!WsjcppCore::fileExists(m_sWorkDir + "/html/index.html")) {
    if (!WsjcppCore::dirExists(m_sWorkDir + "/html")) {
      WsjcppCore::makeDir(m_sWorkDir + "/html");
    }

    WsjcppLog::warn(TAG, "Extracting html/index.html and files");
    const std::vector<WsjcppResourceFile*> &vFiles = WsjcppResourcesManager::list();
    for (int i = 0; i < vFiles.size(); i++) {
      std::string sFilepath = vFiles[i]->getFilename();
      if (sFilepath.rfind("./data_sample/html/", 0) == 0) {
        std::vector<std::string> vPath = WsjcppCore::split(sFilepath, "/");
        vPath.erase (vPath.begin(),vPath.begin()+3);
        std::string sNewFilepath = WsjcppCore::join(vPath, "/");
        sNewFilepath = wsjcpp::normalizeFilePath(m_sWorkDir + "/html/" + sNewFilepath);
        if (!WsjcppCore::fileExists(sNewFilepath)) {
          std::cout << "Extracting file '" << sFilepath << "' to '" << sNewFilepath << "'" << std::endl;
        } else {
          std::cout << "File '" << sNewFilepath << "' already exists. Skip." << std::endl;
          continue;
        }

        // prepare folders
        std::string sFolder = wsjcpp::normalizeFilePath(m_sWorkDir + "/html/");
        for (int p = 0; p < vPath.size()-1; p++) {
          sFolder = wsjcpp::normalizeFilePath(sFolder + "/" + vPath[p]);
          if (!WsjcppCore::dirExists(sFolder)) {
            WsjcppCore::makeDir(sFolder);
          }
        }

        if (!WsjcppCore::writeFile(sNewFilepath, vFiles[i]->getBuffer(), vFiles[i]->getBufferSize())) {
          std::cout << "ERROR. Could not write file. " << std::endl;
          continue;
        } else {
          std::cout << "Successfully created file. " << std::endl;
          if (!WsjcppCore::setFilePermissions(sNewFilepath, WsjcppFilePermissions(0x776), sError)) {
            WsjcppLog::throw_err(TAG, sError);
          }
        }
      }
    }
  }
}

bool EmployConfig::checkYamlMainKeys(WsjcppYaml &yamlConfig) {
  // check main keys
  auto cur = yamlConfig.getCursor();
  // TODO list from vars
  std::vector<std::string> expected_keys = {
    "scoreboard",
    "game",
    "checkers",
    "teams",
  };
  std::vector<std::string> main_keys = cur.keys();
  for (int i = 0; i < main_keys.size(); i++) {
    if (std::find(expected_keys.begin(), expected_keys.end(), main_keys[i]) == expected_keys.end()) {
      WsjcppLog::err(TAG, "Got unexpected key in main: '" + main_keys[i] + "'");
      return false;
    }
  }
  for (int i = 0; i < expected_keys.size(); i++) {
    if (std::find(main_keys.begin(), main_keys.end(), expected_keys[i]) == main_keys.end()) {
      WsjcppLog::err(TAG, "Not found expected key in config: '" + expected_keys[i] + "'");
      return false;
    }
  }
  return true;
}

bool EmployConfig::applyGameConf(WsjcppYaml &yamlConfig) {
  auto cur = yamlConfig.getCursor();
  if (!cur.hasKey("game")) {
      WsjcppLog::err(TAG, "Missing 'game'");
      return false;
  }
  cur = cur["game"];

  std::string err;

  WsjcppLog::info(TAG, "Game start: " + m_game_start_utc->value());
  WsjcppLog::info(TAG, "Game start (UNIX timestamp): " + std::to_string(m_game_start_utc->value_in_seconds()));
  WsjcppLog::info(TAG, "Game end: " + m_game_end_utc->value());
  WsjcppLog::info(TAG, "Game end (UNIX timestamp): " + std::to_string(m_game_end_utc->value_in_seconds()));

  if (m_game_end_utc->value_in_seconds() <= m_game_start_utc->value_in_seconds()) {
    WsjcppLog::err(TAG, "game.end must be gather then game.start");
    return false;
  }

  WsjcppLog::info(TAG, "game.coffee_break_start: " + m_game_coffee_break_start_utc->value());
  WsjcppLog::info(TAG, "Game coffee break start (UNIX timestamp): " + std::to_string(m_game_coffee_break_start_utc->value_in_seconds()));

  WsjcppLog::info(TAG, "game.coffee_break_end: " + m_game_coffee_break_end_utc->value());
  WsjcppLog::info(TAG, "Game coffee break end (UNIX timestamp): " + std::to_string(m_game_coffee_break_end_utc->value_in_seconds()));

  if (m_game_start_utc->value_in_seconds() < m_game_coffee_break_start_utc->value_in_seconds()
    && m_game_coffee_break_start_utc->value_in_seconds() < m_game_end_utc->value_in_seconds()
    && m_game_start_utc->value_in_seconds() < m_game_coffee_break_end_utc->value_in_seconds()
    && m_game_coffee_break_end_utc->value_in_seconds() < m_game_end_utc->value_in_seconds()
  ) {
    WsjcppLog::info(TAG, "Oh! Game has coffee break! nice!");
    m_bHasCoffeeBreak = true;
  }

  return true;
}

bool EmployConfig::applyServicesConfig(WsjcppYaml &yamlConfig) {
  m_vServicesConf.clear();

  WsjcppYamlCursor yamlCheckers = yamlConfig["checkers"];

  if (yamlCheckers.size() == 0) {
    WsjcppLog::err(TAG, "Checkers does not defined");
    return false;
  }

  for (int i = 0; i < yamlCheckers.size(); i++) {
    WsjcppYamlCursor yamlChecker = yamlCheckers[i];

    // default values of service config
    ctf01d::service_config _serviceConf;
    std::string err;

    if (!_serviceConf.read(yamlChecker, m_sWorkDir, err)) {
      WsjcppLog::err(TAG, err);
      return false;
    }

    if (!_serviceConf.isEnabled()) {
      WsjcppLog::warn(TAG, "Checker for service " + _serviceConf.id() + " - disabled ");
      continue;
    }

    for (unsigned int i = 0; i < m_vServicesConf.size(); i++) {
      if (m_vServicesConf[i].id() == _serviceConf.id()) {
        WsjcppLog::err(TAG, "Already registered checker for service '" + _serviceConf.id() + "'");
        return false;
      }
    }

    m_vServicesConf.push_back(_serviceConf);

    // set write permissions for all to directory with checker
    if (!WsjcppCore::setFilePermissions(_serviceConf.scriptDir(), WsjcppFilePermissions(0x777), err)) {
      WsjcppLog::err(TAG, err);
      return false;
    }

    std::string script_absolute_path = wsjcpp::normalizeFilePath(_serviceConf.scriptDir() + "/" + _serviceConf.scriptPath());
    if (!WsjcppCore::fileExists(script_absolute_path)) {
      WsjcppLog::err(TAG, "File " + script_absolute_path + " did not exists");
      return false;
    }
    // set write permissions for all to script of checker
    if (!WsjcppCore::setFilePermissions(script_absolute_path, WsjcppFilePermissions(0x777), err)) {
      WsjcppLog::err(TAG, err);
      return false;
    }

    WsjcppLog::ok(TAG, "Registered checker for service " + _serviceConf.id());
  }

  if (m_vServicesConf.size() == 0) {
    WsjcppLog::err(TAG, "No one defined checkers in config");
    return false;
  }

  return true;
}

bool EmployConfig::readTeamsConf(WsjcppYaml &yamlConfig) {
  m_vTeamsConf.clear();
  EmployTeamLogos *pTeamLogos = findWsjcppEmploy<EmployTeamLogos>();

  WsjcppYamlCursor yamlTeams = yamlConfig["teams"];

  if (yamlTeams.size() == 0) {
    WsjcppLog::err(TAG, "Teams does not defined");
    return false;
  }

  std::vector<std::string> vIPAddresses;

  for (int i = 0; i < yamlTeams.size(); i++) {
    WsjcppYamlCursor yamlTeam = yamlTeams[i];
    std::string sTeamId = yamlTeam["id"].valStr();
    // TODO check sTeamId format

    WsjcppLog::info(TAG, "id = " + sTeamId);
    bool bTeamActive = yamlTeam["active"].valBool();
    WsjcppLog::info(TAG, "active = " + std::string(bTeamActive ? "yes" : "no"));
    if (!bTeamActive) {
      WsjcppLog::warn(TAG, "Team " + sTeamId + " - deactivated");
      continue;
    }

    for (unsigned int i = 0; i < m_vTeamsConf.size(); i++) {
      if (m_vTeamsConf[i].getId() == sTeamId) {
        WsjcppLog::err(TAG, "Already registered team with id " + sTeamId);
        return false;
      }
    }

    std::string sTeamName = yamlTeam["name"].valStr();
    WsjcppLog::info(TAG, "name = " + sTeamName);

    std::string sTeamIpAddress = yamlTeam["ip_address"].valStr();
    WsjcppLog::info(TAG, "ip_address = " + sTeamIpAddress);
    std::string sError;
    if (!isValidIPv4(sTeamIpAddress, sError)) {
      WsjcppLog::err(TAG, "Invalid IPv4 address" + sError);
      return false;
    }

    // Check duplicate IP addresses
    if (std::find(vIPAddresses.begin(), vIPAddresses.end(), sTeamIpAddress) == vIPAddresses.end()) {
      vIPAddresses.push_back(sTeamIpAddress);
    } else {
      WsjcppLog::err(TAG, "Found duplicate IP address: " + sTeamIpAddress);
      return false;
    }

    std::string sTeamLogo = yamlTeam["logo"].valStr();
    sTeamLogo = wsjcpp::normalizeFilePath(m_sWorkDir + "/" + sTeamLogo);
    if (!pTeamLogos->loadTeamLogo(sTeamId, sTeamLogo)) {
      return false;
    }
    WsjcppLog::info(TAG, "logo = " + sTeamLogo);

    // default values of service config
    Ctf01dTeamDef _teamConf;
    _teamConf.setId(sTeamId);
    _teamConf.setName(sTeamName);
    _teamConf.setActive(true);
    _teamConf.setIpAddress(sTeamIpAddress);
    _teamConf.setLogo(sTeamLogo);

    m_vTeamsConf.push_back(_teamConf);
    WsjcppLog::ok(TAG, "Registered team " + sTeamId);
  }

  if (m_vTeamsConf.size() == 0) {
    WsjcppLog::err(TAG, "No one defined team in config");
    return false;
  }

  return true;
}

bool EmployConfig::isValidIPv4(const std::string &sValue, std::string &sError) {
  int n = 0;
  std::string s[4] = {"", "", "", ""};
  for (int i = 0; i < sValue.length(); i++) {
    char c = sValue[i];
    if (n > 3) {
      sError = "Groups number must be less than 5 (like '0.0.0.0')";
      return false;
    }
    if (c >= '0' && c <= '9') {
      s[n] += c;
    } else if (c == '.') {
      n++;
    } else {
      sError = "Unexpected character '";
      sError += c;
      sError += "'";
      return false;
    }
  }
  for (int i = 0; i < 4; i++) {
    if (s[i].length() > 3) {
      sError =
          "Value '" + s[i] + "' could not contains more than 3 digits in a row";
      return false;
    }
    int p = std::stoi(s[i]);
    if (p > 255 || p < 0) {
      sError = "Value '" + std::to_string(p) + "' must be 0..255";
      return false;
    }
  }
  return true;
}

bool EmployConfig::initWorkDir() {
  WsjcppLog::info(TAG, "Work Directory is " + m_sWorkDir);
  std::string sWorkDir = this->getWorkDir();
  if (sWorkDir == "") {
    WsjcppLog::throw_err(TAG, "Work Directory not defined.");
    return false;
  }
  if (!WsjcppCore::dirExists(sWorkDir)) {
    WsjcppLog::err(TAG, "Directory " + sWorkDir + " does not exists");
    return false;
  }
  return true;
}

bool EmployConfig::initLogger() {
  // init logger
  std::string sLogDir = m_sWorkDir + "/logs/" + WsjcppCore::getCurrentTimeForFilename();
  sLogDir = wsjcpp::normalizeFilePath(sLogDir);
  if (!WsjcppCore::dirExists(sLogDir)) {
    if (!WsjcppCore::makeDirsPath(sLogDir)) {
      WsjcppLog::err(TAG, "Could not make dirs for logs: " + sLogDir);
      return false;
    }
    std::string sError;
    if (!WsjcppCore::setFilePermissions(sLogDir, WsjcppFilePermissions(0x776), sError)) {
      WsjcppLog::throw_err(TAG, sError);
    }
  }
  if (!WsjcppCore::dirExists(sLogDir)) {
    std::cout << "Error: Folder '" << sLogDir << "' does not exists and could not created, please check access rights to parent folder.\n";
    return false;
  }
  WsjcppLog::setPrefixLogFile("ctf01d");
  WsjcppLog::setLogDirectory(sLogDir);
  WsjcppLog::setRotationPeriodInSec(600); // every 10 min  // TODO rotation period must be in config.yml
  WsjcppLog::setEnableLogFile(true);
  std::cout << "Logger: '" + sLogDir + "' \n";
  return true;
}
