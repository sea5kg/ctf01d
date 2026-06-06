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
#include <fstream>
#include <iomanip>
#include <wsjcpp_core.h>
#include <wsjcpp_yaml.h>
#include "ctf01d/employees/employ_images.h"
#include "ctf01d/utils/ctf01d_logger.h"
#include "ctf01d/include/ctf01d_globals.h"
#include "ctf01d/include/i_web_server.h"
#include "third_party/smallsha1/smallsha1.h"
#include <sys/stat.h>
#include <stdio.h>

REGISTRY_WSJCPP_EMPLOY(EmployConfig)

EmployConfig::EmployConfig()
: WsjcppEmployBase({ EmployConfig::name() }, {}) {
  TAG = EmployConfig::name();
  m_files_watcher = std::make_shared<ctf01d::files_watcher>();

  // game options
  m_game_id = ctf01d::var_string::create({"game", "id"}, "test", m_game_vars);
  m_game_name = ctf01d::var_string::create({"game", "name"}, "Test", m_game_vars);

  m_bAppliedConfig = false;
  m_flag_lifetime_in_seconds = ctf01d::var_int::create({"game", "flag_lifetime_in_seconds"}, 60, m_game_vars);
  m_flag_lifetime_in_seconds->set_minimum(1);
  m_flag_lifetime_in_seconds->set_maximum(MAX_FLAG_LIFETIME_SECONDS);

  m_flag_cost_in_points = ctf01d::var_int::create({"game", "flag_cost_in_points"}, 100, m_game_vars);
  m_flag_cost_in_points->set_minimum(1);
  m_flag_cost_in_points->set_maximum(MAX_FLAG_COST_IN_POINTS);

  m_game_start_utc = ctf01d::var_datetime::create({"game", "start_utc"}, "2023-11-12 16:00:00", m_game_vars);
  m_game_end_utc = ctf01d::var_datetime::create({"game", "end_utc"}, "2030-11-12 22:00:00", m_game_vars);
  m_game_coffee_break_start_utc = ctf01d::var_datetime::create({"game", "coffee_break_start"}, "2023-11-12 20:00:00", m_game_vars);
  m_game_coffee_break_end_utc = ctf01d::var_datetime::create({"game", "coffee_break_end"}, "2023-11-12 21:00:00", m_game_vars);

  m_has_coffee_break = false;

  // scoreboard config

  m_scoreboard_port = ctf01d::var_int::create({"scoreboard", "port"}, 8080, m_scoreboard_vars);
  m_scoreboard_port->set_minimum(MIN_TCP_PORT);
  m_scoreboard_port->set_maximum(MAX_TCP_PORT);
  m_scoreboard_random = ctf01d::var_bool::create({"scoreboard", "random"}, false, m_scoreboard_vars);
  m_scoreboard_html_folder = ctf01d::var_dir::create({"scoreboard", "html-dir-path"}, "./html", m_sWorkDir, m_scoreboard_vars);
  m_scoreboard_metrics_enabled = ctf01d::var_bool::create({"scoreboard", "prometheus-metrics-endpoint", "enabled"}, false, m_scoreboard_vars);
  m_scoreboard_metrics_allowed_for = ctf01d::var_allowed_ip::create({"scoreboard", "prometheus-metrics-endpoint", "allowed-for"}, "127.0.*", m_scoreboard_vars);

  m_ip_or_host_prefix = ctf01d::var_string::create({"config", "ip-or-host-prefix"}, "", m_teams_config);
  m_ip_or_host_suffix = ctf01d::var_string::create({"config", "ip-or-host-suffix"}, "", m_teams_config);

  m_pScoreboard = nullptr;
}

EmployConfig::~EmployConfig() {
  // TODO cleanup
  m_game_vars.clear();
  m_scoreboard_vars.clear();
}

bool EmployConfig::init(const std::string &sName, bool bSilent) {
  if (!initWorkDir()) {
    return false;
  }

  if (!initLogger()) {
    return false;
  }

  this->update_files_in_data();

  if (!this->applyConfig()) {
    ctf01d::log::err(TAG, "Configuration file has some problems");
    return false;
  }

  return true;
}

bool EmployConfig::deinit(const std::string &sName, bool bSilent) {
  ctf01d::log::info(TAG, "deinit");
  // wait stop threads
  if (m_thread_watcher.joinable()) {
    m_thread_watcher.join();
  }
  return true;
}

void EmployConfig::setWorkDir(const std::string &sWorkDir) {
  if (m_sWorkDir != "" && m_sWorkDir != sWorkDir) {
    std::cout << "Changed work-dir to '" + sWorkDir + "'" << std::endl;
  }
  m_sWorkDir = sWorkDir;
  m_config_filepath = m_sWorkDir + "/config.yml";
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
  ctf01d::log::info(TAG, "Loading configuration...");

  ctf01d::log::info(TAG, "Reading config: " + m_config_filepath);

  if (!WsjcppCore::fileExists(m_config_filepath)) {
    ctf01d::log::err(TAG, "File " + m_config_filepath + " does not exists");
    return false;
  }

  WsjcppYaml yamlConfig;
  std::string sError;
  if (!yamlConfig.loadFromFile(m_config_filepath, sError)) {
    ctf01d::log::err(TAG, "Could not parse " + m_config_filepath + ", reason: " + sError);
    return false;
  }

  if (!checkYamlMainKeys(yamlConfig)) {
    return false;
  }

  auto cursor = yamlConfig.getCursor();
  std::string err;
  if (!m_game_vars.read(cursor, err)) {
    ctf01d::log::err(TAG, err);
    return false;
  }
  if (!m_scoreboard_vars.read(cursor, err)) {
    ctf01d::log::err(TAG, err);
    return false;
  }

  // CTF01D_PORT
  if (!applyScoreboardPortFromEnv()) {
    return false;
  }

  if (!this->checkGameConf()) {
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
  m_files_watcher->watchFile(m_config_filepath);
  m_thread_watcher = std::thread(&EmployConfig::thread_watcher, this);
  return m_bAppliedConfig;
}

std::vector<ctf01d::team_config> &EmployConfig::teamsConf() {
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

std::shared_ptr<ctf01d::var_bool> EmployConfig::scoreboard_metrics_enabled() const {
  return m_scoreboard_metrics_enabled;
}

std::shared_ptr<ctf01d::var_allowed_ip> EmployConfig::scoreboard_metrics_allowed_for() const {
  return m_scoreboard_metrics_allowed_for;
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
  return m_has_coffee_break;
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

// helper
std::string sha1_by_string(const std::string &data) {
  char hexstring[41]; // 40 chars + a zero
  std::memset(hexstring, 0, sizeof hexstring);

  unsigned char hash[20];
  sha1::calc(data.c_str(), data.length(), hash);
  sha1::toHexString(hash, hexstring);
  return std::string(hexstring);
}

std::string sha1_by_data(const char *data, int len) {
  char hexstring[41]; // 40 chars + a zero
  std::memset(hexstring, 0, sizeof hexstring);

  unsigned char hash[20];
  sha1::calc(data, len, hash);
  sha1::toHexString(hash, hexstring);
  return std::string(hexstring);
}

std::string sha1_by_file(const std::string &sFilename) {
  std::ifstream f(sFilename, std::ifstream::binary);
  if (!f) {
    return "Could not open file";
  }
  // get length of file:
  f.seekg (0, f.end);
  int nBufferSize = f.tellg();
  f.seekg (0, f.beg);
  char *pBuffer = new char [nBufferSize];
  // read data as a block:
  f.read(pBuffer, nBufferSize);
  if (!f) {
    delete[] pBuffer;
    // f.close();
    ctf01d::log::throw_err("sha1_by_file", "Could not read file. Only " + std::to_string(f.gcount()) + " could be read");
    return "";
  }
  f.close();
  char hexstring[41]; // 40 chars + a zero
  std::memset(hexstring, 0, sizeof hexstring);
  unsigned char hash[20];
  sha1::calc(pBuffer, nBufferSize, hash);
  sha1::toHexString(hash, hexstring);
  delete[] pBuffer;
  return std::string(hexstring);
}

void EmployConfig::update_files_in_data() {
  std::string sError;
  if (!WsjcppCore::dirExists(m_sWorkDir + "/logs")) {
    WsjcppCore::makeDir(m_sWorkDir + "/logs");
    if (!WsjcppCore::setFilePermissions(m_sWorkDir + "/logs", WsjcppFilePermissions(0x755), sError)) {
      ctf01d::log::throw_err(TAG, sError);
    }
  }

  nlohmann::json previous_files_sha1 = load_files_sha1();

  if (!WsjcppCore::fileExists(m_sWorkDir + "/config.yml")) {
    ctf01d::log::warn(TAG, "Extracting config.yml and files");
    ctf01d::log::warn(TAG, "Extracting checker_example_*");
    const std::vector<WsjcppResourceFile*> &vFiles = WsjcppResourcesManager::list();
    std::vector<std::string> vExecutableFiles;
    for (int i = 0; i < vFiles.size(); i++) {
      std::string filepath = vFiles[i]->getFilename();
      if (filepath.rfind("./data_sample/checker_example_", 0) == 0) {
        std::vector<std::string> vPath = WsjcppCore::split(filepath, "/");
        std::string sDirname = vPath[2];
        vPath.erase (vPath.begin(),vPath.begin()+3);
        std::string sNewFilepath = WsjcppCore::join(vPath, "/");
        sNewFilepath = wsjcpp::normalizeFilePath(m_sWorkDir + "/" + sDirname + "/" + sNewFilepath);
        if (!WsjcppCore::fileExists(sNewFilepath)) {
          std::cout << "Extracting file '" << filepath << "' to '" << sNewFilepath << "'" << std::endl;
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

  update_data_html(previous_files_sha1);
  save_files_sha1(previous_files_sha1);
}

nlohmann::json EmployConfig::load_files_sha1() {
  nlohmann::json files_sha1;
  if (WsjcppCore::fileExists(m_sWorkDir + "/files_sha1.json")) {
    std::ifstream ifs(m_sWorkDir + "/files_sha1.json");
    files_sha1 = nlohmann::json::parse(ifs);
  }
  return files_sha1;
}

void EmployConfig::save_files_sha1(nlohmann::json &files) {
  std::ofstream output(m_sWorkDir + "/files_sha1.json");
  output << std::setw(2) << files << std::endl;
}

void EmployConfig::update_data_html(nlohmann::json &previous_files_sha1) {
  ctf01d::log::warn(TAG, "Updating files in data/html");
  if (!WsjcppCore::dirExists(m_sWorkDir + "/html")) {
    WsjcppCore::makeDir(m_sWorkDir + "/html");
  }
  
  const std::vector<WsjcppResourceFile*> &vFiles = WsjcppResourcesManager::list();
  for (int i = 0; i < vFiles.size(); i++) {
    std::string source_filepath = vFiles[i]->getFilename();
    if (source_filepath.rfind("./data_sample/html/", 0) != 0) {
      continue;
    }
    // remove base folder
    std::vector<std::string> vPath = WsjcppCore::split(source_filepath, "/");
    vPath.erase (vPath.begin(),vPath.begin()+3);
    std::string target_filepath = WsjcppCore::join(vPath, "/");
    target_filepath = wsjcpp::normalizeFilePath(m_sWorkDir + "/html/" + target_filepath);

    // prepare folders
    if (!WsjcppCore::fileExists(target_filepath)) {
      std::string dirpath = wsjcpp::normalizeFilePath(m_sWorkDir + "/html/");
      for (int p = 0; p < vPath.size()-1; p++) {
        dirpath = wsjcpp::normalizeFilePath(dirpath + "/" + vPath[p]);
        if (!WsjcppCore::dirExists(dirpath)) {
          if (!WsjcppCore::makeDir(dirpath)) {
            std::cout << "ERROR. Could not create: " << dirpath << std::endl;
            continue;
          }
        }
      }
    }

    std::string previous_sha1 = "";
    if (previous_files_sha1.contains(source_filepath)) {
      previous_sha1 = previous_files_sha1[source_filepath];
    }

    if (WsjcppCore::fileExists(target_filepath) && previous_sha1 != "") {
      if (previous_sha1 != sha1_by_file(target_filepath)) {
        // Skip. file has changes by user. Skip.
        std::cout << "Warning. Could not override file, because has changes: " << target_filepath << std::endl;
        continue;
      }
    }

    std::string new_sha1 = sha1_by_data(vFiles[i]->getBuffer(), vFiles[i]->getBufferSize());
    if (WsjcppCore::fileExists(target_filepath) && new_sha1 == previous_sha1) {
      // Skip. file has same content
      continue;
    }

    if (!WsjcppCore::writeFile(target_filepath, vFiles[i]->getBuffer(), vFiles[i]->getBufferSize())) {
      std::cout << "ERROR. Could not write/override file. " << std::endl;
      continue;
    }

    std::cout << "Successfully created/updated file: " << target_filepath << std::endl;
    std::string err;
    if (!WsjcppCore::setFilePermissions(target_filepath, WsjcppFilePermissions(0x644), err)) {
      ctf01d::log::throw_err(TAG, err);
    }
    previous_files_sha1[source_filepath] = new_sha1;
  }
}

bool EmployConfig::checkYamlMainKeys(WsjcppYaml &yamlConfig) {
  // check main keys
  auto cur = yamlConfig.getCursor();
  // TODO list from vars
  std::vector<std::string> expected_keys = {
    "scoreboard",
    "game",
    "services",
    "teams",
  };
  std::vector<std::string> main_keys = cur.keys();
  for (int i = 0; i < main_keys.size(); i++) {
    if (std::find(expected_keys.begin(), expected_keys.end(), main_keys[i]) == expected_keys.end()) {
      ctf01d::log::err(TAG, "Got unexpected key in main: '" + main_keys[i] + "'");
      return false;
    }
  }
  for (int i = 0; i < expected_keys.size(); i++) {
    if (std::find(main_keys.begin(), main_keys.end(), expected_keys[i]) == main_keys.end()) {
      ctf01d::log::err(TAG, "Not found expected key in config: '" + expected_keys[i] + "'");
      return false;
    }
  }
  return true;
}

bool EmployConfig::checkGameConf() {
  std::string err;

  ctf01d::log::info(TAG, "Game start: " + m_game_start_utc->value());
  ctf01d::log::info(TAG, "Game start (UNIX timestamp): " + std::to_string(m_game_start_utc->value_in_seconds()));
  ctf01d::log::info(TAG, "Game end: " + m_game_end_utc->value());
  ctf01d::log::info(TAG, "Game end (UNIX timestamp): " + std::to_string(m_game_end_utc->value_in_seconds()));

  if (m_game_end_utc->value_in_seconds() <= m_game_start_utc->value_in_seconds()) {
    ctf01d::log::err(TAG, "game.end must be gather then game.start");
    return false;
  }

  ctf01d::log::info(TAG, "game.coffee_break_start: " + m_game_coffee_break_start_utc->value());
  ctf01d::log::info(TAG, "Game coffee break start (UNIX timestamp): " + std::to_string(m_game_coffee_break_start_utc->value_in_seconds()));

  ctf01d::log::info(TAG, "game.coffee_break_end: " + m_game_coffee_break_end_utc->value());
  ctf01d::log::info(TAG, "Game coffee break end (UNIX timestamp): " + std::to_string(m_game_coffee_break_end_utc->value_in_seconds()));

  if (m_game_start_utc->value_in_seconds() < m_game_coffee_break_start_utc->value_in_seconds()
    && m_game_coffee_break_start_utc->value_in_seconds() < m_game_end_utc->value_in_seconds()
    && m_game_start_utc->value_in_seconds() < m_game_coffee_break_end_utc->value_in_seconds()
    && m_game_coffee_break_end_utc->value_in_seconds() < m_game_end_utc->value_in_seconds()
  ) {
    ctf01d::log::ok(TAG, "Oh! Game has coffee break! nice!");
    m_has_coffee_break = true;
  }
  return true;
}

bool EmployConfig::applyScoreboardPortFromEnv() {
  std::string str_port;
  if (WsjcppCore::getEnv("CTF01D_PORT", str_port)) {
    ctf01d::log::warn(TAG, "CTF01D_PORT='" + str_port + "'");
    try {
      int port = std::stoi(str_port);
      std::string err;
      if (!m_scoreboard_port->set_value(port, err)) {
        ctf01d::log::err(TAG, "CTF01D_PORT='" + str_port + "' is wrong. " + err);
        return false;
      }
    } catch (const std::invalid_argument& e) {
      ctf01d::log::err(TAG, "No conversion could be performed. CTF01D_PORT='" + str_port + "'");
      std::cerr << "Error: \n";
      return false;
    } catch (const std::out_of_range& e) {
      ctf01d::log::err(TAG, "The converted value is too big for an int.. CTF01D_PORT='" + str_port + "'");
      return false;
    } catch (...) {
      ctf01d::log::err(TAG, "The converted value is too big for an int.. CTF01D_PORT='" + str_port + "'");
      return false;
    }
    ctf01d::log::info(TAG, "scoreboard.port will be overridden from environment variable. CTF01D_PORT='" + str_port + "'");
    return true;
  }
  return true;
}

bool EmployConfig::applyServicesConfig(WsjcppYaml &yamlConfig) {
  m_vServicesConf.clear();
  auto images = findWsjcppEmploy<EmployImages>();

  WsjcppYamlCursor yamlCheckers = yamlConfig["services"];

  if (yamlCheckers.size() == 0) {
    ctf01d::log::err(TAG, "Checkers does not defined");
    return false;
  }

  for (int i = 0; i < yamlCheckers.size(); i++) {
    WsjcppYamlCursor yamlChecker = yamlCheckers[i];

    // default values of service config
    ctf01d::service_config _serviceConf;
    std::string err;

    if (!_serviceConf.read(yamlChecker, m_sWorkDir, err)) {
      ctf01d::log::err(TAG, err);
      return false;
    }

    if (!_serviceConf.is_enabled()) {
      ctf01d::log::warn(TAG, "Checker for service " + _serviceConf.id() + " - disabled ");
      continue;
    }

    for (unsigned int i = 0; i < m_vServicesConf.size(); i++) {
      if (m_vServicesConf[i].id() == _serviceConf.id()) {
        ctf01d::log::err(TAG, "Already registered checker for service '" + _serviceConf.id() + "'");
        return false;
      }
    }

    if (!images->load_service_logo(_serviceConf.id(), _serviceConf.logo_path())) {
      return false;
    }
    ctf01d::log::info(TAG, "Loaded service logo = " + _serviceConf.logo_path());
    if (!images->load_service_big_logo(_serviceConf.id(), _serviceConf.logo_big_path())) {
      return false;
    }
    ctf01d::log::info(TAG, "Loaded service logo-big = " + _serviceConf.logo_big_path());

    m_vServicesConf.push_back(_serviceConf);

    // set write permissions for all to directory with checker
    if (!WsjcppCore::setFilePermissions(_serviceConf.script_dir(), WsjcppFilePermissions(0x777), err)) {
      ctf01d::log::err(TAG, err);
      return false;
    }

    std::string script_absolute_path = wsjcpp::normalizeFilePath(_serviceConf.script_dir() + "/" + _serviceConf.script_path());
    if (!WsjcppCore::fileExists(script_absolute_path)) {
      ctf01d::log::err(TAG, "File " + script_absolute_path + " did not exists");
      return false;
    }
    // set write permissions for all to script of checker
    if (!WsjcppCore::setFilePermissions(script_absolute_path, WsjcppFilePermissions(0x777), err)) {
      ctf01d::log::err(TAG, err);
      return false;
    }

    ctf01d::log::ok(TAG, "Registered checker for service " + _serviceConf.id());
  }

  if (m_vServicesConf.size() == 0) {
    ctf01d::log::err(TAG, "No one defined services in config");
    return false;
  }

  return true;
}

bool EmployConfig::readTeamsConf(WsjcppYaml &yamlConfig) {
  m_vTeamsConf.clear();
  auto images = findWsjcppEmploy<EmployImages>();

  WsjcppYamlCursor cursor = yamlConfig["teams"];
  std::string err;
  if (!m_teams_config.read(cursor, err)) {
    ctf01d::log::err(TAG, err);
    return false;
  }

  if (!cursor.hasKey("list")) {
    ctf01d::log::err(TAG, "Missing teams.list");
    return false;
  }
  cursor = cursor["list"];

  if (cursor.size() == 0) {
    ctf01d::log::err(TAG, "Teams does not defined");
    return false;
  }

  std::vector<std::string> ip_or_host_teams;

  for (int i = 0; i < cursor.size(); i++) {
    WsjcppYamlCursor cur = cursor[i];
    ctf01d::team_config _team_config;
    _team_config.set_ip_or_host_prefix(m_ip_or_host_prefix->value());
    _team_config.set_ip_or_host_suffix(m_ip_or_host_suffix->value());

    std::string err;
    if (!_team_config.read(cur, m_sWorkDir, err)) {
      return false;
    }

    // TODO check sTeamId format
    if (!_team_config.is_active()) {
      ctf01d::log::warn(TAG, "Team " + _team_config.id() + " - deactivated");
      continue;
    }

    for (unsigned int i = 0; i < m_vTeamsConf.size(); i++) {
      if (m_vTeamsConf[i].id() == _team_config.id()) {
        ctf01d::log::err(TAG, "Already registered team with id " + _team_config.id());
        return false;
      }
    }

    // Check duplicate IP addresses
    if (std::find(ip_or_host_teams.begin(), ip_or_host_teams.end(), _team_config.ip_or_host()) == ip_or_host_teams.end()) {
      ip_or_host_teams.push_back(_team_config.ip_or_host());
    } else {
      ctf01d::log::err(TAG, "Found duplicate IP or Host address: " + _team_config.ip_or_host());
      return false;
    }

    if (!images->load_team_logo(_team_config.id(), _team_config.logo_path())) {
      return false;
    }
    ctf01d::log::info(TAG, "Loaded team logo = " + _team_config.logo_path());
    if (!images->load_team_big_logo(_team_config.id(), _team_config.logo_big_path())) {
      return false;
    }
    ctf01d::log::info(TAG, "Loaded team logo-big = " + _team_config.logo_path());

    m_vTeamsConf.push_back(_team_config);
    ctf01d::log::ok(TAG, "Registered team " + _team_config.id());
  }

  if (m_vTeamsConf.size() == 0) {
    ctf01d::log::err(TAG, "No one defined team in config");
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
  ctf01d::log::info(TAG, "Work Directory is " + m_sWorkDir);
  std::string sWorkDir = this->getWorkDir();
  if (sWorkDir == "") {
    ctf01d::log::throw_err(TAG, "Work Directory not defined.");
    return false;
  }
  if (!WsjcppCore::dirExists(sWorkDir)) {
    ctf01d::log::err(TAG, "Directory " + sWorkDir + " does not exists");
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
      ctf01d::log::err(TAG, "Could not make dirs for logs: " + sLogDir);
      return false;
    }
    std::string sError;
    if (!WsjcppCore::setFilePermissions(sLogDir, WsjcppFilePermissions(0x776), sError)) {
      ctf01d::log::throw_err(TAG, sError);
    }
  }
  if (!WsjcppCore::dirExists(sLogDir)) {
    std::cout << "Error: Folder '" << sLogDir << "' does not exists and could not created, please check access rights to parent folder.\n";
    return false;
  }
  ctf01d::log::set_log_filename_prefix("ctf01d");
  ctf01d::log::set_log_directory(sLogDir);
  ctf01d::log::set_rotation_period_in_seconds(600); // every 10 min  // TODO rotation period must be in config.yml
  ctf01d::log::set_enable_log_file(true);
  std::cout << "Logger: '" + sLogDir + "' \n";
  return true;
}

void EmployConfig::thread_watcher() {

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    std::map<std::string, long> modified_files = m_files_watcher->get_modified_files();
    if (modified_files.size() == 0) { // nothing changes
      continue;
    }
    ctf01d::log::info(TAG, "Watcher thread found changes");

    // TODO images/logos update

    std::scoped_lock lock(m_mutex_thread_watcher);

    for (auto it = modified_files.begin(); it != modified_files.end(); ++it) {
      const std::string &filepath = it->first;
      if (filepath == m_config_filepath) {
        hot_reload_config_yaml();
      } else {
        ctf01d::log::warn(TAG, "TODO update file watched " + filepath);
      }
    }
  }
}

void EmployConfig::hot_reload_config_yaml() {
  if (!WsjcppCore::fileExists(m_config_filepath)) {
    ctf01d::log::err(TAG, "File " + m_config_filepath + " does not exists");
    return;
  }
  WsjcppYaml yamlConfig;
  std::string err;
  if (!yamlConfig.loadFromFile(m_config_filepath, err)) {
    ctf01d::log::err(TAG, "Could not parse " + m_config_filepath + ", reason: " + err);
    return;
  }
  auto cursor = yamlConfig.getCursor();
  {
    bool prev_value = m_scoreboard_metrics_enabled->value();
    if (m_scoreboard_metrics_enabled->read(cursor, err)) {
      if (prev_value != m_scoreboard_metrics_enabled->value()) {
        ctf01d::log::info(TAG, "Updated option: " + m_scoreboard_metrics_enabled->name() + " " + m_scoreboard_metrics_enabled->to_string());
        findWsjcppEmploy<IWebServer>()->set_metrics_enabled(m_scoreboard_metrics_enabled->value());
      }
    };
  }

  // std::shared_ptr<ctf01d::var_allowed_ip> m_scoreboard_metrics_allowed_for;

  // if (!m_scoreboard_vars.read(cursor, err)) {
  //   ctf01d::log::err(TAG, err);
  //   return;
  // }
}
