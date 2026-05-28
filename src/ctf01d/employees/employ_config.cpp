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
    m_bAppliedConfig = false;
    m_nFlagLifetimeInMin = 10;
    m_nScoreboardPort = 8080;
    m_bScoreboardRandom = false;
    m_pScoreboard = nullptr;

    m_nGameStartUTCInSec = 0;
    m_nGameEndUTCInSec = 0;
    m_bHasCoffeeBreak = false;
    m_sGameCoffeeBreakStart = "";
    m_sGameCoffeeBreakEnd = "";
    m_nGameCoffeeBreakStartUTCInSec = 0;
    m_nGameCoffeeBreakEndUTCInSec = 0;
    m_nBasicCostsStolenFlagInPoints = 10;
    m_nCostDefenseFlagInPoints10 = 10; // default 1.0
}

EmployConfig::~EmployConfig() {
    // TODO cleanup
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
    m_sScoreboardHtmlFolder = m_sWorkDir + "/html"; // default value
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

    // apply the game config
    if (!this->applyGameConf(yamlConfig)) {
        return false;
    }

    if (!this->applyCheckersConf(yamlConfig)) {
        return false;
    }

    if (!this->readTeamsConf(yamlConfig)) {
        return false;
    }

    // apply the scoreboard config
    if (!this->applyScoreboardConf(yamlConfig)) {
        return false;
    }

    // scoreboard
    m_pScoreboard = std::make_shared<Ctf01dScoreboard>(
        m_bScoreboardRandom,
        m_nGameStartUTCInSec,
        m_nGameEndUTCInSec,
        m_nGameCoffeeBreakStartUTCInSec,
        m_nGameCoffeeBreakEndUTCInSec
    );

    m_bAppliedConfig = true;
    return m_bAppliedConfig;
}

std::vector<Ctf01dTeamDef> &EmployConfig::teamsConf() {
    return m_vTeamsConf;
}

std::vector<Ctf01dServiceDef> &EmployConfig::servicesConf() {
    return m_vServicesConf;
}

int EmployConfig::scoreboardPort() const {
    return m_nScoreboardPort;
}

std::string EmployConfig::scoreboardHtmlFolder() const {
    return m_sScoreboardHtmlFolder;
}

bool EmployConfig::scoreboardRandom() const {
    return m_bScoreboardRandom;
}

std::string EmployConfig::gameId() const {
    return m_sGameId;
}

std::string EmployConfig::gameName() const  {
    return m_sGameName;
}

int EmployConfig::flagTimeliveInMin() const  {
    return m_nFlagLifetimeInMin;
}

int EmployConfig::getBasicCostsStolenFlagInPoints() const {
    return m_nBasicCostsStolenFlagInPoints;
}

int EmployConfig::getCostDefenseFlagInPoints10() const {
    return m_nCostDefenseFlagInPoints10;
}

int EmployConfig::gameStartUTCInSec() const {
    return m_nGameStartUTCInSec;
}

int EmployConfig::gameEndUTCInSec() const {
    return m_nGameEndUTCInSec;
}

bool EmployConfig::gameHasCoffeeBreak() {
    return m_bHasCoffeeBreak;
}

int EmployConfig::gameCoffeeBreakStartUTCInSec() {
    return m_nGameCoffeeBreakStartUTCInSec;
}

int EmployConfig::gameCoffeeBreakEndUTCInSec() {
    return m_nGameCoffeeBreakEndUTCInSec;
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
                    if (chmod(sNewFilepath.c_str(), S_IRWXU|S_IRWXG) != 0) {
                        std::cout << "ERROR. Could not change permissions for. " << std::endl;
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

bool EmployConfig::applyGameConf(WsjcppYaml &yamlConfig) {

    m_sGameId = yamlConfig["game"]["id"].valStr();
    WsjcppLog::info(TAG, "game.id: " + m_sGameId);
    m_sGameName = yamlConfig["game"]["name"].valStr();
    WsjcppLog::info(TAG, "game.name: " + m_sGameName);

    m_sGameStart = yamlConfig["game"]["start"].valStr();
    WsjcppLog::info(TAG, "game.start: " + m_sGameStart);
    {
        std::istringstream in{m_sGameStart.c_str()};
        date::sys_seconds tp;
        in >> date::parse("%Y-%m-%d %T", tp);
        m_nGameStartUTCInSec = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
    }

    WsjcppLog::info(TAG, "Game start (UNIX timestamp): " + std::to_string(m_nGameStartUTCInSec));

    m_sGameEnd = yamlConfig["game"]["end"].valStr();
    WsjcppLog::info(TAG, "game.end: " + m_sGameEnd);
    {
        std::istringstream in{m_sGameEnd.c_str()};
        date::sys_seconds tp;
        in >> date::parse("%Y-%m-%d %T", tp);
        m_nGameEndUTCInSec = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
    }
    WsjcppLog::info(TAG, "Game end (UNIX timestamp): " + std::to_string(m_nGameEndUTCInSec));

    m_nFlagLifetimeInMin = yamlConfig["game"]["flag_lifetime_in_min"].valInt();
    WsjcppLog::info(TAG, "game.flag_lifetime_in_min: " + std::to_string(m_nFlagLifetimeInMin));

    m_nBasicCostsStolenFlagInPoints = yamlConfig["game"]["basic_costs_stolen_flag_in_points"].valInt();
    WsjcppLog::info(TAG, "game.basic_costs_stolen_flag_in_points: " + std::to_string(m_nBasicCostsStolenFlagInPoints));

    m_nCostDefenseFlagInPoints10 = std::atof(yamlConfig["game"]["cost_defense_flag_in_points"].valStr().c_str())*10;
    WsjcppLog::info(TAG, "game.cost_defense_flag_in_points (*10): " + std::to_string(m_nCostDefenseFlagInPoints10));

    if (m_nGameStartUTCInSec == 0) {
        WsjcppLog::err(TAG, "game.start - not found");
        return false;
    }

    if (m_nGameEndUTCInSec == 0) {
        WsjcppLog::err(TAG, "game.end - not found");
        return false;
    }

    if (m_nGameEndUTCInSec < m_nGameStartUTCInSec) {
        WsjcppLog::err(TAG, "game.end must be gather then game.start");
        return false;
    }

    if (m_nFlagLifetimeInMin <= 0) {
        WsjcppLog::err(TAG, "game.flag_lifetime_in_min could not be less than 0");
        return false;
    }

    if (m_nFlagLifetimeInMin > MAX_FLAG_LIFETIME_MINUTES) {
        WsjcppLog::err(TAG, "game.flag_lifetime_in_min could not be gather than " + std::to_string(MAX_FLAG_LIFETIME_MINUTES));
        return false;
    }

    m_sGameCoffeeBreakStart = yamlConfig["game"]["coffee_break_start"].valStr();
    WsjcppLog::info(TAG, "game.coffee_break_start: " + m_sGameCoffeeBreakStart);
    {
        std::istringstream in{m_sGameCoffeeBreakStart.c_str()};
        date::sys_seconds tp;
        in >> date::parse("%Y-%m-%d %T", tp);
        m_nGameCoffeeBreakStartUTCInSec = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
    }
    WsjcppLog::info(TAG, "Game coffee break start (UNIX timestamp): " + std::to_string(m_nGameCoffeeBreakStartUTCInSec));

    m_sGameCoffeeBreakEnd = yamlConfig["game"]["coffee_break_end"].valStr();
    WsjcppLog::info(TAG, "game.coffee_break_end: " + m_sGameCoffeeBreakEnd);
    {
        std::istringstream in{m_sGameCoffeeBreakEnd.c_str()};
        date::sys_seconds tp;
        in >> date::parse("%Y-%m-%d %T", tp);
        m_nGameCoffeeBreakEndUTCInSec = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
    }
    WsjcppLog::info(TAG, "Game coffee break start (UNIX timestamp): " + std::to_string(m_nGameCoffeeBreakEndUTCInSec));

    if (m_nGameStartUTCInSec < m_nGameCoffeeBreakStartUTCInSec
        && m_nGameCoffeeBreakStartUTCInSec < m_nGameEndUTCInSec
        && m_nGameStartUTCInSec < m_nGameCoffeeBreakEndUTCInSec
        && m_nGameCoffeeBreakEndUTCInSec < m_nGameEndUTCInSec
    ) {
        WsjcppLog::info(TAG, "Oh! Game has coffee break! nice!");
        m_bHasCoffeeBreak = true;
    }

    if (m_nBasicCostsStolenFlagInPoints <= 0) {
        WsjcppLog::err(TAG, "game.basic_costs_stolen_flag_in_points could not be less than 0");
        return false;
    }

    if (m_nBasicCostsStolenFlagInPoints > 500) {
        WsjcppLog::err(TAG, "game.basic_costs_stolen_flag_in_points could not be gather than 500");
        return false;
    }

    return true;
}

bool EmployConfig::applyScoreboardConf(WsjcppYaml &yamlConfig) {

    m_nScoreboardPort = yamlConfig["scoreboard"]["port"].valInt();
    if (m_nScoreboardPort <= 10 || m_nScoreboardPort > 65536) {
        WsjcppLog::err(TAG, "wrong scoreboard.port (expected value od 11..65535)");
        return false;
    }
    WsjcppLog::info(TAG, "scoreboard.port: " + std::to_string(m_nScoreboardPort));

    m_bScoreboardRandom = yamlConfig["scoreboard"]["random"].valBool();
    WsjcppLog::info(TAG, "scoreboard.random: " + std::string(m_bScoreboardRandom == true ? "yes" : "no"));

    m_sScoreboardHtmlFolder = yamlConfig["scoreboard"]["htmlfolder"].valStr();
    if (m_sScoreboardHtmlFolder.length() > 0) {
        if (m_sScoreboardHtmlFolder[0] != '/') {
            m_sScoreboardHtmlFolder = m_sWorkDir + "/" + m_sScoreboardHtmlFolder;
        }
    } else {
        m_sScoreboardHtmlFolder = m_sWorkDir + "/html";
    }
    m_sScoreboardHtmlFolder = wsjcpp::normalizeFilePath(m_sScoreboardHtmlFolder);

    WsjcppLog::info(TAG, "scoreboard.htmlfolder: " + m_sScoreboardHtmlFolder);

    if (!WsjcppCore::dirExists(m_sScoreboardHtmlFolder)) {
        WsjcppLog::err(TAG, "Directory '" + m_sScoreboardHtmlFolder + "' with scoreboard does not exists");
        return false;
    }

    return true;
}

bool EmployConfig::applyCheckersConf(WsjcppYaml &yamlConfig) {
    m_vServicesConf.clear();

    WsjcppYamlCursor yamlCheckers = yamlConfig["checkers"];

    if (yamlCheckers.size() == 0) {
        WsjcppLog::err(TAG, "Checkers does not defined");
        return false;
    }

    for (int i = 0; i < yamlCheckers.size(); i++) {
        WsjcppYamlCursor yamlChecker = yamlCheckers[i];
        std::string sServiceId = yamlChecker["id"].valStr();


        // std::string sServiceConfPath = m_sWorkspaceDir + "/checker_" + sServiceId + "/service.conf";

        std::string sServiceName = yamlChecker["service_name"].valStr();
        WsjcppLog::info(TAG, "service_name = " + sServiceName);

        bool bServiceEnable = yamlChecker["enabled"].valBool();
        WsjcppLog::info(TAG, "enabled = " + std::string(bServiceEnable ? "yes" : "no"));

        std::string sServiceScriptPath = yamlChecker["script_path"].valStr();
        WsjcppLog::info(TAG, "script_path = " + sServiceScriptPath);
        std::string sServiceScriptDir = m_sWorkDir + "/checker_" + sServiceId + "/";
        if (!WsjcppCore::dirExists(sServiceScriptDir)) {
            WsjcppLog::err(TAG, "Folder " + sServiceScriptDir + " did not exists");
            return false;
        }
        // set write permissions for all to directory with checker
        std::string sError;
        if (!WsjcppCore::setFilePermissions(sServiceScriptDir, WsjcppFilePermissions(0x777), sError)) {
            WsjcppLog::err(TAG, sError);
            return false;
        }

        WsjcppLog::info(TAG, "sServiceScriptDir: " + sServiceScriptDir);
        if (!WsjcppCore::fileExists(sServiceScriptDir + sServiceScriptPath)) {
            WsjcppLog::err(TAG, "File " + sServiceScriptPath + " did not exists");
            return false;
        }
        // set write permissions for all to script of checker
        if (!WsjcppCore::setFilePermissions(sServiceScriptDir + sServiceScriptPath, WsjcppFilePermissions(0x777), sError)) {
            WsjcppLog::err(TAG, sError);
            return false;
        }

        int nServiceScriptWait = yamlChecker["script_wait_in_sec"].valInt();
        WsjcppLog::info(TAG, "script_wait_in_sec = " + std::to_string(nServiceScriptWait));

        if (nServiceScriptWait < 5) {
            WsjcppLog::err(TAG, "Could not parse script_wait_in_sec - must be more than 4 sec ");
            return false;
        }

        int nServiceSleepBetweenRun = yamlChecker["time_sleep_between_run_scripts_in_sec"].valInt();
        WsjcppLog::info(TAG, "time_sleep_between_run_scripts_in_sec = " + std::to_string(nServiceSleepBetweenRun));

        if (nServiceSleepBetweenRun < nServiceScriptWait*3) {
            WsjcppLog::err(TAG, "Could not parse time_sleep_between_run_scripts_in_sec - must be more than " + std::to_string(nServiceScriptWait*3-1) + " sec ");
            return false;
        }

        if (!bServiceEnable) {
            WsjcppLog::warn(TAG, "Checker for service " + sServiceId + " - disabled ");
            continue;
        }

        for (unsigned int i = 0; i < m_vServicesConf.size(); i++) {
            if (m_vServicesConf[i].id() == sServiceId) {
                WsjcppLog::err(TAG, "Already registered checker for service " + sServiceId);
                return false;
            }
        }

        // default values of service config
        Ctf01dServiceDef _serviceConf;
        _serviceConf.setId(sServiceId);
        _serviceConf.setName(sServiceName);
        _serviceConf.setScriptPath(sServiceScriptPath);
        _serviceConf.setScriptDir(sServiceScriptDir);
        _serviceConf.setEnabled(bServiceEnable);
        _serviceConf.setScriptWaitInSec(nServiceScriptWait);
        _serviceConf.setTimeSleepBetweenRunScriptsInSec(nServiceSleepBetweenRun);
        m_vServicesConf.push_back(_serviceConf);

        WsjcppLog::ok(TAG, "Registered checker for service " + sServiceId);
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
