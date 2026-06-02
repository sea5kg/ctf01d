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

#include "ctf01d_scoreboard.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <wsjcpp_core.h>
#include "ctf01d/employees/employ_config.h"
#include "ctf01d/employees/employ_flags.h"

Ctf01dScoreboard::Ctf01dScoreboard(
  bool bRandom,
  int nGameStartInSec,
  int nGameEndInSec,
  int nGameCoffeeBreakStartInSec,
  int nGameCoffeeBreakEndInSec
) {
  TAG = "Ctf01dScoreboard";
  auto *config = findWsjcppEmploy<EmployConfig>();
  m_pDatabase = findWsjcppEmploy<EmployDatabase>();
  const std::vector<ctf01d::team_config> &vTeamsConf = config->teamsConf();
  const std::vector<ctf01d::service_config> &vServicesConf = config->servicesConf();
  m_bRandom = bRandom;
  std::string sScoreboardRandom = "Scoreboard random: ";
  sScoreboardRandom = sScoreboardRandom + (m_bRandom ? "yes" : "no");
  WsjcppLog::warn(TAG, sScoreboardRandom);
  std::srand(unsigned(std::time(0)));
  m_nGameStartInSec = nGameStartInSec;
  m_nGameEndInSec = nGameEndInSec;
  m_nGameCoffeeBreakStartInSec = nGameCoffeeBreakStartInSec;
  m_nGameCoffeeBreakEndInSec = nGameCoffeeBreakEndInSec;
  m_nAllDefenseFlags = 0;
  m_nAllTriesActivities = 0;
  m_flag_cost_in_points = config->get_flag_cost_in_points();
  m_nTeamCount = vTeamsConf.size();
  m_pEmployFlags = findWsjcppEmploy<EmployFlags>();
  m_formulas = std::make_shared<Ctf01dFormulasForPoints_RuCtf>();

  m_mapTeamsStatuses.clear(); // possible memory leak
  for (unsigned int i_team = 0; i_team < vTeamsConf.size(); ++i_team) {
    ctf01d::team_config teamConf = vTeamsConf[i_team];
    std::string sTeamId = teamConf.id();
    m_mapTeamsStatuses[sTeamId] = new Ctf01dTeamStatusRow(sTeamId, nGameStartInSec, nGameEndInSec);
    m_mapTeamsStatuses[sTeamId]->setPlace(i_team + 1);
    // random values of service for testing
    if (m_bRandom) {
        int nPoints = (std::rand() % 10000);
        m_mapTeamsStatuses[sTeamId]->setPoints(nPoints);
    }
    for (unsigned int iservice = 0; iservice < vServicesConf.size(); iservice++) {
      ctf01d::service_config service = vServicesConf[iservice];
      m_mapTeamsStatuses[sTeamId]->setServiceStatus(service.id(), Ctf01dServiceStatusCell::SERVICE_DOWN);
      // random states of service for testing
      if (m_bRandom) {
        m_mapTeamsStatuses[sTeamId]->setServiceStatus(service.id(), randomServiceStatus());
        m_mapTeamsStatuses[sTeamId]->setTries(std::rand() % 1000);
      }
    }
  }

  // keep the list of the services ids
  for (unsigned int i = 0; i < vServicesConf.size(); i++) {
    std::string sServiceId = vServicesConf[i].id();
    m_mapServiceCostsAndStatistics[sServiceId] = new Ctf01dServiceStatistics(sServiceId);
  }

  initJsonScoreboard();
}

void Ctf01dScoreboard::initJsonScoreboard() {
  std::lock_guard<std::mutex> lock(m_mutexJson);
  m_jsonScoreboard.clear();
  EmployConfig *pConfig = findWsjcppEmploy<EmployConfig>();
  const std::vector<ctf01d::team_config> &vTeamsConf = pConfig->teamsConf();
  const std::vector<ctf01d::service_config> &vServices = pConfig->servicesConf();

  nlohmann::json jsonServicesStatistics;
  for (unsigned int iservice = 0; iservice < vServices.size(); iservice++) {
    ctf01d::service_config serviceConf = vServices[iservice];
    nlohmann::json serviceStatistics;
    m_mapServiceCostsAndStatistics[serviceConf.id()]->updateJsonServiceStatistics(serviceStatistics);
    jsonServicesStatistics[serviceConf.id()] = serviceStatistics;
  }
  m_jsonScoreboard["s_sta"] = jsonServicesStatistics;
  m_jsonScoreboard["sum_act"] = m_nAllTriesActivities;

  nlohmann::json jsonScoreboard;
  for (unsigned int i_team = 0; i_team < vTeamsConf.size(); ++i_team) {
    ctf01d::team_config teamConf = vTeamsConf[i_team];
    std::string sTeamId = teamConf.id();
    nlohmann::json teamData;
    teamData["place"] = m_mapTeamsStatuses[sTeamId]->getPlace();
    teamData["points"] = double(m_mapTeamsStatuses[sTeamId]->getPoints());
    teamData["tries"] = 0;
    teamData["logo_last_updated"] = 0;
    nlohmann::json jsonServices;
    for (unsigned int iservice = 0; iservice < vServices.size(); iservice++) {
      ctf01d::service_config serviceConf = vServices[iservice];
      nlohmann::json serviceData;
      serviceData["def"] = 0;
      serviceData["pt_def"] = 0;
      serviceData["att"] = 0;
      serviceData["pt_att"] = 0;
      serviceData["sla"] = 100;
      serviceData["status"] = m_mapTeamsStatuses[sTeamId]->serviceStatus(serviceConf.id());
      jsonServices[serviceConf.id()] = serviceData;
    }
    teamData["ts_sta"] = jsonServices;
    jsonScoreboard[teamConf.id()] = teamData;
  }
  m_jsonScoreboard["scoreboard"] = jsonScoreboard;
  nlohmann::json jsonGame;
  jsonGame["t0"] = pConfig->gameStartUTCInSec();
  jsonGame["t1"] = pConfig->gameCoffeeBreakStartUTCInSec();
  jsonGame["t2"] = pConfig->gameCoffeeBreakEndUTCInSec();
  jsonGame["t3"] = pConfig->gameEndUTCInSec();
  jsonGame["tc"] = WsjcppCore::getCurrentTimeInSeconds();
  m_jsonScoreboard["game"] = jsonGame;
}

void Ctf01dScoreboard::updateJsonScoreboard() {
  std::lock_guard<std::mutex> lock(m_mutexJson);
  // TODO update score
  // TODO update costs
}

std::string Ctf01dScoreboard::randomServiceStatus() {
  std::string sResult = Ctf01dServiceStatusCell::SERVICE_DOWN;
  int nState = std::rand() % 5;
  switch (nState) {
    case 0: sResult = Ctf01dServiceStatusCell::SERVICE_UP; break;
    case 1: sResult = Ctf01dServiceStatusCell::SERVICE_DOWN; break;
    case 2: sResult = Ctf01dServiceStatusCell::SERVICE_MUMBLE; break;
    case 3: sResult = Ctf01dServiceStatusCell::SERVICE_CORRUPT; break;
    case 4: sResult = Ctf01dServiceStatusCell::SERVICE_SHIT; break;
  }
  return sResult;
}

void Ctf01dScoreboard::setServiceStatus(const std::string &sTeamId, const std::string &sServiceId, const std::string &sStatus) {
  std::lock_guard<std::mutex> lock(m_mutexJson);
  std::string sNewStatus = m_bRandom ? randomServiceStatus() : sStatus;

  std::map<std::string, Ctf01dTeamStatusRow *>::iterator it;
  it = m_mapTeamsStatuses.find(sTeamId);
  if (it != m_mapTeamsStatuses.end()) {
    if (it->second->serviceStatus(sServiceId) != sNewStatus) {
      it->second->setServiceStatus(sServiceId, sNewStatus);
      m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["status"] = sNewStatus;
    }
  }
}

void Ctf01dScoreboard::incrementTries(const std::string &sTeamId) {
  std::lock_guard<std::mutex> lock(m_mutexJson);
  std::map<std::string, Ctf01dTeamStatusRow *>::iterator it;
  m_nAllTriesActivities++;
  it = m_mapTeamsStatuses.find(sTeamId);
  if (it != m_mapTeamsStatuses.end()) {
    it->second->setTries(it->second->tries() + 1);
    m_jsonScoreboard["scoreboard"][sTeamId]["tries"] = it->second->tries();
  }
  m_jsonScoreboard["sum_act"] = m_nAllTriesActivities;
}

void Ctf01dScoreboard::initStateFromStorage() {
  EmployConfig *pConfig = findWsjcppEmploy<EmployConfig>();
  const std::vector<ctf01d::service_config> &vServices = pConfig->servicesConf();

  WsjcppLog::info(TAG, "Loading alive flags...");
  std::vector<Ctf01dFlag> vFlagLives = m_pDatabase->listOfLiveFlags();
  for (unsigned int i = 0; i < vFlagLives.size(); i++) {
    Ctf01dFlag flag = vFlagLives[i];
    m_mapFlagsLive[flag.getValue()] = flag;
  }

  // load services statistics
  WsjcppLog::info(TAG, "Loading services statistics...");
  m_nAllDefenseFlags = 0;
  struct FlagsForService {
    std::string sServiceID;
    std::string sFirstBloodTeamID;
    long nFirstBloodTime;
    int nStolenFlags;
    int nDefenseFlags;
  };
  std::vector<FlagsForService> vFlags;
  for (unsigned int i = 0; i < vServices.size(); i++) {
    std::string sServiceID = vServices[i].id();
    FlagsForService f;
    f.sServiceID = sServiceID;
    f.nStolenFlags = m_pDatabase->numberOfStolenFlagsForService(sServiceID);
    f.nDefenseFlags = m_pDatabase->numberOfDefenseFlagForService(sServiceID);
    if (f.nStolenFlags > 0) {
      std::pair<std::string, long> fb = m_pDatabase->getFirstBloodFromStolenFlagsForService(sServiceID);
      f.sFirstBloodTeamID = fb.first;
      f.nFirstBloodTime = fb.second;
    }
    m_nAllDefenseFlags += f.nDefenseFlags;
    vFlags.push_back(f);
  }

  WsjcppLog::info(TAG, "Setting services statistics...");
  for (int i = 0; i < vFlags.size(); i++) {
    FlagsForService f = vFlags[i];
    m_mapServiceCostsAndStatistics[f.sServiceID]->setStolenFlagsForService(f.nStolenFlags);
    m_mapServiceCostsAndStatistics[f.sServiceID]->setDefenseFlagsForService(f.nDefenseFlags);
    if (f.nStolenFlags > 0) {
      m_mapServiceCostsAndStatistics[f.sServiceID]->setFirstBloodTeamId(f.sFirstBloodTeamID, f.nFirstBloodTime);
    }
  }

  WsjcppLog::info(TAG, "Setting teams statistics...");
  m_nAllTriesActivities = 0;
  std::map<std::string, Ctf01dTeamStatusRow *>::iterator it;
  for (it = m_mapTeamsStatuses.begin(); it != m_mapTeamsStatuses.end(); it++) {
    Ctf01dTeamStatusRow *pRow = it->second;

    int nTries = m_pDatabase->numberOfFlagAttempts(pRow->teamId());
    m_nAllTriesActivities += nTries;
    pRow->setTries(nTries);
    m_jsonScoreboard["scoreboard"][pRow->teamId()]["tries"] = nTries;

    for (unsigned int i = 0; i < vServices.size(); i++) {
      std::string sServiceID = vServices[i].id();

      // calculate defense
      int nDefenseFlags = m_pDatabase->numberOfFlagsDefense(pRow->teamId(), sServiceID);
      int nDefensePoints = m_pDatabase->sumPointsOfFlagsDefense(pRow->teamId(), sServiceID);
      pRow->setServiceDefenseFlagsAndPoints(sServiceID, nDefenseFlags, nDefensePoints);
      m_jsonScoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["def"] = nDefenseFlags;
      m_jsonScoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["pt_def"] = double(nDefensePoints);

      // calculate attack
      int nAttackFlags = m_pDatabase->numberOfFlagsStollen(pRow->teamId(), sServiceID);
      int nAttackPoints = m_pDatabase->sumPointsOfFlagsStollen(pRow->teamId(), sServiceID);
      pRow->setServiceAttackFlagsAndPoints(sServiceID, nAttackFlags, nAttackPoints);
      m_jsonScoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["att"] = nAttackFlags;
      m_jsonScoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["pt_att"] = double(nAttackPoints);

      // calculate uptime / sla
      int nPutsFlagsAllResults = m_pDatabase->numberOfFlagFlagsCheckerPutAllResults(pRow->teamId(), sServiceID);
      int nPutsFlagsSuccessResults = m_pDatabase->numberOfFlagFlagsCheckerPutSuccessResult(pRow->teamId(), sServiceID);
      pRow->setServiceFlagsForCalculateSLA(sServiceID, nPutsFlagsAllResults, nPutsFlagsSuccessResults);
      m_jsonScoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["sla"] = pRow->calculateSLA(sServiceID);
    }
  }
  m_jsonScoreboard["sum_act"] = m_nAllTriesActivities;

  WsjcppLog::info(TAG, "Sorting places and apply to json...");
  {
    std::lock_guard<std::mutex> lock(m_mutexJson);
    sortPlaces();
    updateServicesStatistics();
  }
}

std::optional<int> Ctf01dScoreboard::incrementAttackScore(const Ctf01dFlag &flag, const std::string &sTeamId) {
  std::lock_guard<std::mutex> lock(m_mutexJson);
  if (m_pDatabase->isAlreadyStole(flag, sTeamId)) {
    return std::nullopt;
  }
  std::string sServiceId = flag.getServiceId();

  // TODO calculate
  // int nFlagPoints = m_mapServiceCostsAndStatistics[sServiceId]->getCostStolenFlag()*10; // one number after dot
  int flag_points = m_flag_cost_in_points->value(); // TODO basic
  int nDateAction = WsjcppCore::getCurrentTimeInMilliseconds();
  // victim place in scoreboard
  std::map<std::string, Ctf01dTeamStatusRow *>::iterator it_victim;
  it_victim = m_mapTeamsStatuses.find(flag.getTeamId());
  int victim_place = 0;
  if (it_victim != m_mapTeamsStatuses.end()) {
    victim_place = it_victim->second->getPlace();
  }

  std::map<std::string, Ctf01dTeamStatusRow *>::iterator it;
  it = m_mapTeamsStatuses.find(sTeamId);
  if (it != m_mapTeamsStatuses.end()) {
    Ctf01dTeamStatusRow *pRow = it->second;
    int thief_place = pRow->getPlace();
    flag_points = m_formulas->calcStolen(flag_points, victim_place, thief_place, m_nTeamCount);

    m_pDatabase->insertToFlagsStolen(flag, sTeamId, flag_points, nDateAction, victim_place, thief_place);
    pRow->incrementAttack(sServiceId, flag_points);
    pRow->updatePoints();
    m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["att"] = pRow->getAttackFlags(sServiceId);
    m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["pt_att"] = pRow->getAttackPoints(sServiceId);
    m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["sla"] = pRow->calculateSLA(sServiceId);
    m_jsonScoreboard["scoreboard"][sTeamId]["points"] = double(pRow->getPoints());
    sortPlaces();
  }

  std::map<std::string, Ctf01dServiceStatistics *>::iterator it2;
  it2 = m_mapServiceCostsAndStatistics.find(sServiceId);
  if (it2 != m_mapServiceCostsAndStatistics.end()) {
    if (it2->second->getFirstBloodTeamId() == "?") {
      it2->second->setFirstBloodTeamId(sTeamId, nDateAction);
    }
    updateServicesStatistics();
  }
  return std::optional<int>(flag_points);
}

void Ctf01dScoreboard::incrementDefenseScore(const Ctf01dFlag &flag) {
  std::lock_guard<std::mutex> lock(m_mutexJson);

  std::string sTeamId = flag.getTeamId();
  std::string sServiceId = flag.getServiceId();
  int nFlagPoints = m_flag_cost_in_points->value();
  m_pDatabase->insertToFlagsDefense(flag, nFlagPoints);

  std::map<std::string, Ctf01dTeamStatusRow *>::iterator it;
  it = m_mapTeamsStatuses.find(sTeamId);
  if (it != m_mapTeamsStatuses.end()) {
    Ctf01dTeamStatusRow *pRow = it->second;
    pRow->incrementDefense(sServiceId, nFlagPoints);
    m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["def"] = pRow->getDefenseFlags(sServiceId);
    m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["pt_def"] = pRow->getDefensePoints(sServiceId);
    m_jsonScoreboard["scoreboard"][sTeamId]["points"] = pRow->getPoints();
    sortPlaces();
  }

  // TODO call Employ Scoreboard
  std::map<std::string, Ctf01dServiceStatistics *>::iterator it2;
  it2 = m_mapServiceCostsAndStatistics.find(sServiceId);
  if (it2 != m_mapServiceCostsAndStatistics.end()) {
    m_nAllDefenseFlags++;
    it2->second->doIncrementDefenseFlagsForService();
    updateServicesStatistics();
  }
}

void Ctf01dScoreboard::incrementFlagsPuttedAndServiceUp(const Ctf01dFlag &flag) {
  std::string sServiceId = flag.getServiceId();
  std::string sTeamId = flag.getTeamId();
  std::string sNewStatus = m_bRandom ? randomServiceStatus() : Ctf01dServiceStatusCell::SERVICE_UP;

  // insert flag lives
  {
    std::lock_guard<std::mutex> lock(m_mutexFlagsLive);
    std::map<std::string, Ctf01dFlag>::iterator it;
    it = m_mapFlagsLive.find(flag.getValue());
    if (it != m_mapFlagsLive.end()) {
      WsjcppLog::err(TAG, flag.getValue() + " - flag already exists");
    } else {
      m_mapFlagsLive[flag.getValue()] = flag;
      m_pDatabase->insertToFlagLive(flag);
      m_pDatabase->insertToFlagsCheckerPutResult(flag, "up");
      m_mapTeamsStatuses[flag.getTeamId()]->incrementPutFlagSuccess(flag.getServiceId());
    }
  }

  // success putted
  std::lock_guard<std::mutex> lock(m_mutexJson);
  std::map<std::string, Ctf01dTeamStatusRow *>::iterator it;
  it = m_mapTeamsStatuses.find(sTeamId);
  if (it != m_mapTeamsStatuses.end()) {
    Ctf01dTeamStatusRow *pRow = it->second;
    if (pRow->serviceStatus(sServiceId) != sNewStatus) {
      pRow->setServiceStatus(sServiceId, sNewStatus);
    }
    pRow->updatePoints();
    m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["status"] = sNewStatus;
    m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["sla"] = pRow->calculateSLA(sServiceId);
    m_jsonScoreboard["scoreboard"][sTeamId]["points"] = pRow->getPoints();
    sortPlaces();
    updateServicesStatistics();
  }
}

void Ctf01dScoreboard::insertFlagPutFail(const Ctf01dFlag &flag, const std::string &sServiceStatus, const std::string &sDescrStatus) {
  m_pDatabase->insertToFlagsCheckerPutResult(flag, sDescrStatus);

  std::lock_guard<std::mutex> lock(m_mutexJson);

  std::string sServiceId = flag.getServiceId();
  std::string sTeamId = flag.getTeamId();
  std::string sNewStatus = m_bRandom ? randomServiceStatus() : sServiceStatus;

  std::map<std::string, Ctf01dTeamStatusRow *>::iterator it;
  it = m_mapTeamsStatuses.find(flag.getTeamId());
  if (it != m_mapTeamsStatuses.end()) {
    Ctf01dTeamStatusRow *pRow = it->second;
    if (pRow->serviceStatus(sServiceId) != sNewStatus) {
      pRow->setServiceStatus(sServiceId, sNewStatus);
    }
    pRow->incrementPutFlagFail(sServiceId);
    pRow->updatePoints();
    m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["status"] = sNewStatus;
    m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["sla"] = pRow->calculateSLA(sServiceId);
    m_jsonScoreboard["scoreboard"][sTeamId]["points"] = pRow->getPoints();
    sortPlaces();
  }
}

void Ctf01dScoreboard::updateScore(const std::string &sTeamId, const std::string &sServiceId) {
  std::lock_guard<std::mutex> lock(m_mutexJson);
  std::map<std::string,Ctf01dTeamStatusRow *>::iterator it;
  it = m_mapTeamsStatuses.find(sTeamId);
  if (it != m_mapTeamsStatuses.end()) {
    Ctf01dTeamStatusRow *pRow = it->second;
    // pRow->updateScore(sServiceId);
    m_jsonScoreboard["scoreboard"][sTeamId]["points"] = pRow->getPoints();
    sortPlaces();
  }
}

std::string Ctf01dScoreboard::serviceStatus(const std::string &sTeamId, const std::string &sServiceId) {
  std::map<std::string, Ctf01dTeamStatusRow *>::iterator it;
  it = m_mapTeamsStatuses.find(sTeamId);
  if (it != m_mapTeamsStatuses.end()) {
    return it->second->serviceStatus(sServiceId);
  }
  return "";
}

static bool sort_using_greater_than(double u, double v) {
  return u > v;
}

void Ctf01dScoreboard::sortPlaces() {
  // std::lock_guard<std::mutex> lock(m_mutexJson);
  // sort places
  {
    std::vector<int> vScores;
    std::map<std::string, Ctf01dTeamStatusRow *>::iterator it1;
    for (it1 = m_mapTeamsStatuses.begin(); it1 != m_mapTeamsStatuses.end(); it1++) {
      if(std::find(vScores.begin(), vScores.end(), it1->second->getPoints()) == vScores.end()) {
        vScores.push_back(it1->second->getPoints());
      }
    }
    std::sort(vScores.begin(), vScores.end(), sort_using_greater_than);
    for (it1 = m_mapTeamsStatuses.begin(); it1 != m_mapTeamsStatuses.end(); it1++) {
      int nPoints = it1->second->getPoints();
      ptrdiff_t pos = std::find(vScores.begin(), vScores.end(), nPoints) - vScores.begin();
      it1->second->setPlace(pos + 1); // TODO fix: same scores will be same place
    }
  }

  // update json
  {
    std::map<std::string, Ctf01dTeamStatusRow *>::iterator it1;
    for (it1 = m_mapTeamsStatuses.begin(); it1 != m_mapTeamsStatuses.end(); it1++) {
      Ctf01dTeamStatusRow *pTeamStatus = it1->second;
      std::string sTeamId_ = pTeamStatus->teamId();

      // std::cout << sTeamNum << ": result: score: " << pTeamStatus->score() << ", place: " << pTeamStatus->getPlace() << "\n";
      m_jsonScoreboard["scoreboard"][sTeamId_]["points"] = pTeamStatus->getPoints();
      m_jsonScoreboard["scoreboard"][sTeamId_]["place"] = pTeamStatus->getPlace();
      m_jsonScoreboard["scoreboard"][sTeamId_]["tries"] = pTeamStatus->tries();
    }
  }
}

void Ctf01dScoreboard::updateServicesStatistics() {
  // std::lock_guard<std::mutex> lock(m_mutexJson);
  // TODO update costs
  std::map<std::string, Ctf01dServiceStatistics *>::iterator it1;

  // nlohmann::json jsonCosts;
  for (it1 = m_mapServiceCostsAndStatistics.begin(); it1 != m_mapServiceCostsAndStatistics.end(); it1++) {
    std::string sId = it1->first;
    it1->second->updateJsonServiceStatistics(m_jsonScoreboard["s_sta"][sId]);
  }
}

std::vector<Ctf01dFlag> Ctf01dScoreboard::outdatedFlagsLive(const std::string &sTeamId, const std::string &sServiceId) {
  std::lock_guard<std::mutex> lock(m_mutexFlagsLive);
  std::vector<Ctf01dFlag> vResult;
  long nCurrentTime = WsjcppCore::getCurrentTimeInMilliseconds();
  std::map<std::string,Ctf01dFlag>::iterator it;
  for (it = m_mapFlagsLive.begin(); it != m_mapFlagsLive.end(); it++) {
    Ctf01dFlag flag = it->second;
    if (flag.getTeamId() == sTeamId
      && flag.getServiceId() == sServiceId
      && flag.getTimeEndInMs() < nCurrentTime
    ) {
      vResult.push_back(flag);
    }
  }
  return vResult;
}

bool Ctf01dScoreboard::findFlagLive(const std::string &sFlagValue, Ctf01dFlag &flag) {
  std::lock_guard<std::mutex> lock(m_mutexFlagsLive);
  std::map<std::string,Ctf01dFlag>::iterator it = m_mapFlagsLive.find(sFlagValue);
  if (it != m_mapFlagsLive.end()) {
    flag.copyFrom(it->second);
    return true;
  }
  return false;
}

void Ctf01dScoreboard::removeFlagLive(const Ctf01dFlag &flag) {
  std::lock_guard<std::mutex> lock(m_mutexFlagsLive);
  std::map<std::string,Ctf01dFlag>::iterator it;
  it = m_mapFlagsLive.find(flag.getValue());
  if (it != m_mapFlagsLive.end()) {
    m_mapFlagsLive.erase(it);
    m_pDatabase->deleteFlagLive(flag);
  } else {
    WsjcppLog::warn(TAG, flag.getValue() + " - flag did not exists");
  }
}

int Ctf01dScoreboard::countFlagsLive() {
  std::lock_guard<std::mutex> lock(m_mutexFlagsLive);
  return static_cast<int>(m_mapFlagsLive.size());
}

std::string Ctf01dScoreboard::toString(){
  std::lock_guard<std::mutex> lock(m_mutexFlagsLive);
  std::string sResult = "";
  std::map<std::string, Ctf01dTeamStatusRow *>::iterator it;
  for (it = m_mapTeamsStatuses.begin(); it != m_mapTeamsStatuses.end(); ++it){
    sResult += it->first + ": \n"
      "\tpoints: " + std::to_string(it->second->getPoints()) + "\n"
      + it->second->servicesToString() + "\n";
  }
  return sResult;
}

const nlohmann::json &Ctf01dScoreboard::toJson(){
  std::lock_guard<std::mutex> lock(m_mutexJson);
  m_jsonScoreboard["game"]["tc"] = WsjcppCore::getCurrentTimeInSeconds();
  return m_jsonScoreboard;
}
