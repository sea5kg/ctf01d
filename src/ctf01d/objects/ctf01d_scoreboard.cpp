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
#include "ctf01d/utils/ctf01d_logger.h"

namespace ctf01d {

scoreboard::scoreboard(
  bool random,
  int game_start_in_seconds,
  int game_end_in_seconds,
  int game_coffee_break_start_in_seconds,
  int game_coffee_break_end_in_seconds
) {
  TAG = "scoreboard";
  auto *config = findWsjcppEmploy<EmployConfig>();
  m_database = findWsjcppEmploy<EmployDatabase>();
  const std::vector<ctf01d::team_config> &vTeamsConf = config->teamsConf();
  const std::vector<ctf01d::service_config> &vServicesConf = config->servicesConf();
  m_random = random;
  std::string scoreboard_random = "Scoreboard random: ";
  scoreboard_random = scoreboard_random + (m_random ? "yes" : "no");
  ctf01d::log::warn(TAG, scoreboard_random);
  std::srand(unsigned(std::time(0)));
  m_game_start_in_seconds = game_start_in_seconds;
  m_game_end_in_seconds = game_end_in_seconds;
  m_game_coffee_break_start_in_seconds = game_coffee_break_start_in_seconds;
  m_game_coffee_break_end_in_seconds = game_coffee_break_end_in_seconds;
  m_all_defense_flags = 0;
  m_all_tries_activities = 0;
  m_flag_cost_in_points = config->get_flag_cost_in_points();
  m_team_count = vTeamsConf.size();
  m_alive_flags = findWsjcppEmploy<alive_flags>();
  m_formulas = std::make_shared<ctf01d::formulas_for_points_ructf>();

  m_teams_statuses.clear(); // possible memory leak
  for (unsigned int i_team = 0; i_team < vTeamsConf.size(); ++i_team) {
    ctf01d::team_config teamConf = vTeamsConf[i_team];
    std::string team_id = teamConf.id();
    m_teams_statuses[team_id] = new ctf01d::team_status_row(team_id, m_game_start_in_seconds, m_game_end_in_seconds);
    m_teams_statuses[team_id]->setPlace(i_team + 1);
    // random values of service for testing
    if (m_random) {
        int nPoints = (std::rand() % 10000);
        m_teams_statuses[team_id]->setPoints(nPoints);
    }
    for (unsigned int iservice = 0; iservice < vServicesConf.size(); iservice++) {
      ctf01d::service_config service = vServicesConf[iservice];
      m_teams_statuses[team_id]->setServiceStatus(service.id(), ctf01d::service_status_cell::SERVICE_DOWN);
      // random states of service for testing
      if (m_random) {
        m_teams_statuses[team_id]->setServiceStatus(service.id(), random_service_status());
        m_teams_statuses[team_id]->setTries(std::rand() % 1000);
      }
    }
  }

  // keep the list of the services ids
  for (unsigned int i = 0; i < vServicesConf.size(); i++) {
    std::string service_id = vServicesConf[i].id();
    m_service_costs_and_statistics[service_id] = new ctf01d::service_statistics(service_id);
  }

  init_json_scoreboard();
}

void scoreboard::init_json_scoreboard() {
  std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
  m_scoreboard.clear();
  EmployConfig *pConfig = findWsjcppEmploy<EmployConfig>();
  const std::vector<ctf01d::team_config> &vTeamsConf = pConfig->teamsConf();
  const std::vector<ctf01d::service_config> &vServices = pConfig->servicesConf();

  nlohmann::json jsonServicesStatistics;
  for (unsigned int iservice = 0; iservice < vServices.size(); iservice++) {
    ctf01d::service_config serviceConf = vServices[iservice];
    nlohmann::json serviceStatistics;
    m_service_costs_and_statistics[serviceConf.id()]->updateJsonServiceStatistics(serviceStatistics);
    jsonServicesStatistics[serviceConf.id()] = serviceStatistics;
  }
  m_scoreboard["s_sta"] = jsonServicesStatistics;
  m_scoreboard["sum_act"] = m_all_tries_activities;

  nlohmann::json jsonScoreboard;
  for (unsigned int i_team = 0; i_team < vTeamsConf.size(); ++i_team) {
    ctf01d::team_config teamConf = vTeamsConf[i_team];
    std::string team_id = teamConf.id();
    nlohmann::json teamData;
    teamData["place"] = m_teams_statuses[team_id]->getPlace();
    teamData["points"] = m_teams_statuses[team_id]->getPoints();
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
      serviceData["status"] = m_teams_statuses[team_id]->serviceStatus(serviceConf.id());
      jsonServices[serviceConf.id()] = serviceData;
    }
    teamData["ts_sta"] = jsonServices;
    jsonScoreboard[teamConf.id()] = teamData;
  }
  m_scoreboard["scoreboard"] = jsonScoreboard;
  nlohmann::json jsonGame;
  jsonGame["t0"] = pConfig->gameStartUTCInSec();
  jsonGame["t1"] = pConfig->gameCoffeeBreakStartUTCInSec();
  jsonGame["t2"] = pConfig->gameCoffeeBreakEndUTCInSec();
  jsonGame["t3"] = pConfig->gameEndUTCInSec();
  jsonGame["tc"] = WsjcppCore::getCurrentTimeInSeconds();
  m_scoreboard["game"] = jsonGame;
}

void scoreboard::update_json_scoreboard() {
  std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
  // TODO update score
  // TODO update costs
}

std::string scoreboard::random_service_status() {
  std::string sResult = ctf01d::service_status_cell::SERVICE_DOWN;
  int nState = std::rand() % 5;
  switch (nState) {
    case 0: sResult = ctf01d::service_status_cell::SERVICE_UP; break;
    case 1: sResult = ctf01d::service_status_cell::SERVICE_DOWN; break;
    case 2: sResult = ctf01d::service_status_cell::SERVICE_MUMBLE; break;
    case 3: sResult = ctf01d::service_status_cell::SERVICE_CORRUPT; break;
    case 4: sResult = ctf01d::service_status_cell::SERVICE_SHIT; break;
  }
  return sResult;
}

void scoreboard::set_service_status(const std::string &team_id, const std::string &service_id, const std::string &sStatus) {
  std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
  std::string sNewStatus = m_random ? random_service_status() : sStatus;

  std::map<std::string, ctf01d::team_status_row *>::iterator it;
  it = m_teams_statuses.find(team_id);
  if (it != m_teams_statuses.end()) {
    if (it->second->serviceStatus(service_id) != sNewStatus) {
      it->second->setServiceStatus(service_id, sNewStatus);
      m_scoreboard["scoreboard"][team_id]["ts_sta"][service_id]["status"] = sNewStatus;
    }
  }
}

void scoreboard::increment_tries(const std::string &team_id) {
  std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
  std::map<std::string, ctf01d::team_status_row *>::iterator it;
  m_all_tries_activities++;
  it = m_teams_statuses.find(team_id);
  if (it != m_teams_statuses.end()) {
    it->second->setTries(it->second->tries() + 1);
    m_scoreboard["scoreboard"][team_id]["tries"] = it->second->tries();
  }
  m_scoreboard["sum_act"] = m_all_tries_activities;
}

void scoreboard::init_state_from_storage() {
  EmployConfig *pConfig = findWsjcppEmploy<EmployConfig>();
  const std::vector<ctf01d::service_config> &vServices = pConfig->servicesConf();

  // load services statistics
  ctf01d::log::info(TAG, "Loading services statistics...");
  m_all_defense_flags = 0;
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
    f.nStolenFlags = m_database->numberOfStolenFlagsForService(sServiceID);
    f.nDefenseFlags = m_database->numberOfDefenseFlagForService(sServiceID);
    if (f.nStolenFlags > 0) {
      std::pair<std::string, long> fb = m_database->getFirstBloodFromStolenFlagsForService(sServiceID);
      f.sFirstBloodTeamID = fb.first;
      f.nFirstBloodTime = fb.second;
    }
    m_all_defense_flags += f.nDefenseFlags;
    vFlags.push_back(f);
  }

  ctf01d::log::info(TAG, "Setting services statistics...");
  for (int i = 0; i < vFlags.size(); i++) {
    FlagsForService f = vFlags[i];
    m_service_costs_and_statistics[f.sServiceID]->setStolenFlagsForService(f.nStolenFlags);
    m_service_costs_and_statistics[f.sServiceID]->setDefenseFlagsForService(f.nDefenseFlags);
    if (f.nStolenFlags > 0) {
      m_service_costs_and_statistics[f.sServiceID]->setFirstBloodTeamId(f.sFirstBloodTeamID, f.nFirstBloodTime);
    }
  }

  ctf01d::log::info(TAG, "Setting teams statistics...");
  m_all_tries_activities = 0;
  std::map<std::string, ctf01d::team_status_row *>::iterator it;
  for (it = m_teams_statuses.begin(); it != m_teams_statuses.end(); it++) {
    ctf01d::team_status_row *pRow = it->second;

    int nTries = m_database->numberOfFlagAttempts(pRow->teamId());
    m_all_tries_activities += nTries;
    pRow->setTries(nTries);
    m_scoreboard["scoreboard"][pRow->teamId()]["tries"] = nTries;

    for (unsigned int i = 0; i < vServices.size(); i++) {
      std::string sServiceID = vServices[i].id();

      // calculate defense
      ctf01d::log::info(TAG, "   -> (" + pRow->teamId() + ") calculate defense");
      int nDefenseFlags = m_database->numberOfFlagsDefense(pRow->teamId(), sServiceID);
      int nDefensePoints = m_database->sumPointsOfFlagsDefense(pRow->teamId(), sServiceID);
      pRow->setServiceDefenseFlagsAndPoints(sServiceID, nDefenseFlags, nDefensePoints);
      m_scoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["def"] = nDefenseFlags;
      m_scoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["pt_def"] = nDefensePoints;

      // calculate attack
      ctf01d::log::info(TAG, "   -> (" + pRow->teamId() + ") calculate attack and flags stollen");
      int nAttackFlags = m_database->numberOfFlagsStollen(pRow->teamId(), sServiceID);
      int nFlagsStollen = -1 * m_database->numberOfFlagsStollenByVictim(pRow->teamId(), sServiceID);
      int nAttackPoints = m_database->sumPointsOfFlagsStollen(pRow->teamId(), sServiceID);
      pRow->setServiceAttackFlagsAndPoints(sServiceID, nAttackFlags, nAttackPoints);
      pRow->setFlagsStollen(sServiceID, nFlagsStollen);
      m_scoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["att_st"] = nFlagsStollen;
      m_scoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["att"] = nAttackFlags;
      m_scoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["pt_att"] = nAttackPoints;

      // calculate uptime / sla
      ctf01d::log::info(TAG, "   -> (" + pRow->teamId() + ") uptime / sla");
      int nPutsFlagsAllResults = m_database->numberOfFlagFlagsCheckerPutAllResults(pRow->teamId(), sServiceID);
      int nPutsFlagsSuccessResults = m_database->numberOfFlagFlagsCheckerPutSuccessResult(pRow->teamId(), sServiceID);
      pRow->setServiceFlagsForCalculateSLA(sServiceID, nPutsFlagsAllResults, nPutsFlagsSuccessResults);
      m_scoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["sla"] = pRow->calculateSLA(sServiceID);
    }
  }
  m_scoreboard["sum_act"] = m_all_tries_activities;

  ctf01d::log::info(TAG, "Sorting places and apply to json...");
  {
    std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
    sort_places();
    update_services_statistics();
  }
}

std::optional<int> scoreboard::increment_attack_score(const ctf01d::flag &flag, const std::string &team_id) {
  std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
  if (m_database->isAlreadyStole(flag, team_id)) {
    return std::nullopt;
  }
  std::string service_id = flag.getServiceId();

  // TODO calculate
  // int nFlagPoints = m_service_costs_and_statistics[service_id]->getCostStolenFlag()*10; // one number after dot
  int flag_points = m_flag_cost_in_points->value(); // TODO basic
  long nDateAction = WsjcppCore::getCurrentTimeInMilliseconds();
  // victim place in scoreboard
  std::map<std::string, ctf01d::team_status_row *>::iterator it_victim;
  it_victim = m_teams_statuses.find(flag.getTeamId());
  int victim_place = 0;
  ctf01d::team_status_row *row_victim = nullptr;
  if (it_victim != m_teams_statuses.end()) {
    row_victim = it_victim->second;
    victim_place = row_victim->getPlace();
  }

  std::map<std::string, ctf01d::team_status_row *>::iterator it;
  it = m_teams_statuses.find(team_id);
  if (it != m_teams_statuses.end()) {
    ctf01d::team_status_row *pRow = it->second;
    int thief_place = pRow->getPlace();
    flag_points = m_formulas->calc_stolen(flag_points, victim_place, thief_place, m_team_count);
    m_database->insertToFlagsStolen(flag, team_id, flag_points, nDateAction, victim_place, thief_place);
    pRow->incrementAttack(service_id, flag_points);
    pRow->updatePoints();
    if (row_victim != nullptr) {
      row_victim->decrementFlagStollen(service_id);
      m_scoreboard["scoreboard"][flag.getTeamId()]["ts_sta"][service_id]["att_st"] = row_victim->getFlagsStollen(service_id);
    }
    m_scoreboard["scoreboard"][team_id]["ts_sta"][service_id]["att"] = pRow->getAttackFlags(service_id);
    m_scoreboard["scoreboard"][team_id]["ts_sta"][service_id]["pt_att"] = pRow->getAttackPoints(service_id);
    m_scoreboard["scoreboard"][team_id]["ts_sta"][service_id]["sla"] = pRow->calculateSLA(service_id);
    m_scoreboard["scoreboard"][team_id]["points"] = pRow->getPoints();
    sort_places();
  }

  std::map<std::string, ctf01d::service_statistics *>::iterator it2;
  it2 = m_service_costs_and_statistics.find(service_id);
  if (it2 != m_service_costs_and_statistics.end()) {
    it2->second->doIncrementStolenFlagsForService();
    if (it2->second->getFirstBloodTeamId() == "?") {
      it2->second->setFirstBloodTeamId(team_id, nDateAction);
    }
    update_services_statistics();
  }
  return std::optional<int>(flag_points);
}

void scoreboard::increment_defense_score(const ctf01d::flag &flag) {
  std::lock_guard<std::mutex> lock(m_mutex_scoreboard);

  std::string team_id = flag.getTeamId();
  std::string service_id = flag.getServiceId();
  int flag_points = m_flag_cost_in_points->value();
  m_database->insertToFlagsDefense(flag, flag_points);

  std::map<std::string, ctf01d::team_status_row *>::iterator it;
  it = m_teams_statuses.find(team_id);
  if (it != m_teams_statuses.end()) {
    ctf01d::team_status_row *pRow = it->second;
    pRow->incrementDefense(service_id, flag_points);
    m_scoreboard["scoreboard"][team_id]["ts_sta"][service_id]["def"] = pRow->getDefenseFlags(service_id);
    m_scoreboard["scoreboard"][team_id]["ts_sta"][service_id]["pt_def"] = pRow->getDefensePoints(service_id);
    m_scoreboard["scoreboard"][team_id]["points"] = pRow->getPoints();
    sort_places();
  }

  // TODO call Employ Scoreboard
  std::map<std::string, ctf01d::service_statistics *>::iterator it2;
  it2 = m_service_costs_and_statistics.find(service_id);
  if (it2 != m_service_costs_and_statistics.end()) {
    m_all_defense_flags++;
    it2->second->doIncrementDefenseFlagsForService();
    update_services_statistics();
  }
}

void scoreboard::increment_flags_putted_and_service_up(const ctf01d::flag &flag) {
  std::string service_id = flag.getServiceId();
  std::string team_id = flag.getTeamId();
  std::string sNewStatus = m_random ? random_service_status() : ctf01d::service_status_cell::SERVICE_UP;

  if (m_alive_flags->insert_alive_flag(flag)) {
    // m_database->insertToFlagLive(flag);
    m_database->insertToFlagsCheckerPutResult(flag, "up");
    m_teams_statuses[flag.getTeamId()]->incrementPutFlagSuccess(flag.getServiceId());
  }

  // success putted
  std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
  std::map<std::string, ctf01d::team_status_row *>::iterator it;
  it = m_teams_statuses.find(team_id);
  if (it != m_teams_statuses.end()) {
    ctf01d::team_status_row *pRow = it->second;
    if (pRow->serviceStatus(service_id) != sNewStatus) {
      pRow->setServiceStatus(service_id, sNewStatus);
    }
    pRow->updatePoints();
    m_scoreboard["scoreboard"][team_id]["ts_sta"][service_id]["status"] = sNewStatus;
    m_scoreboard["scoreboard"][team_id]["ts_sta"][service_id]["sla"] = pRow->calculateSLA(service_id);
    m_scoreboard["scoreboard"][team_id]["points"] = pRow->getPoints();
    sort_places();
    update_services_statistics();
  }
}

void scoreboard::insert_flag_put_fail(const ctf01d::flag &flag, const std::string &service_status, const std::string &descr_status) {
  m_database->insertToFlagsCheckerPutResult(flag, descr_status);

  std::lock_guard<std::mutex> lock(m_mutex_scoreboard);

  std::string service_id = flag.getServiceId();
  std::string team_id = flag.getTeamId();
  std::string sNewStatus = m_random ? random_service_status() : service_status;

  std::map<std::string, ctf01d::team_status_row *>::iterator it;
  it = m_teams_statuses.find(flag.getTeamId());
  if (it != m_teams_statuses.end()) {
    ctf01d::team_status_row *pRow = it->second;
    if (pRow->serviceStatus(service_id) != sNewStatus) {
      pRow->setServiceStatus(service_id, sNewStatus);
    }
    pRow->incrementPutFlagFail(service_id);
    pRow->updatePoints();
    m_scoreboard["scoreboard"][team_id]["ts_sta"][service_id]["status"] = sNewStatus;
    m_scoreboard["scoreboard"][team_id]["ts_sta"][service_id]["sla"] = pRow->calculateSLA(service_id);
    m_scoreboard["scoreboard"][team_id]["points"] = pRow->getPoints();
    sort_places();
  }
}

// void scoreboard::update_points(const std::string &team_id, const std::string &service_id) {
//   std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
//   std::map<std::string,ctf01d::team_status_row *>::iterator it;
//   it = m_teams_statuses.find(team_id);
//   if (it != m_teams_statuses.end()) {
//     ctf01d::team_status_row *pRow = it->second;
//     // pRow->update_score(service_id);
//     m_scoreboard["scoreboard"][team_id]["points"] = pRow->getPoints();
//     sort_places();
//   }
// }

std::string scoreboard::service_status(const std::string &team_id, const std::string &service_id) {
  std::map<std::string, ctf01d::team_status_row *>::iterator it;
  it = m_teams_statuses.find(team_id);
  if (it != m_teams_statuses.end()) {
    return it->second->serviceStatus(service_id);
  }
  return "";
}

static bool sort_using_greater_than(double u, double v) {
  return u > v;
}

void scoreboard::sort_places() {
  // std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
  // sort places
  {
    std::vector<int> vScores;
    std::map<std::string, ctf01d::team_status_row *>::iterator it1;
    for (it1 = m_teams_statuses.begin(); it1 != m_teams_statuses.end(); it1++) {
      if(std::find(vScores.begin(), vScores.end(), it1->second->getPoints()) == vScores.end()) {
        vScores.push_back(it1->second->getPoints());
      }
    }
    std::sort(vScores.begin(), vScores.end(), sort_using_greater_than);
    for (it1 = m_teams_statuses.begin(); it1 != m_teams_statuses.end(); it1++) {
      int nPoints = it1->second->getPoints();
      ptrdiff_t pos = std::find(vScores.begin(), vScores.end(), nPoints) - vScores.begin();
      it1->second->setPlace(pos + 1); // TODO fix: same scores will be same place
    }
  }

  // update json
  {
    std::map<std::string, ctf01d::team_status_row *>::iterator it1;
    for (it1 = m_teams_statuses.begin(); it1 != m_teams_statuses.end(); it1++) {
      ctf01d::team_status_row *pTeamStatus = it1->second;
      std::string team_id_ = pTeamStatus->teamId();

      // std::cout << sTeamNum << ": result: score: " << pTeamStatus->score() << ", place: " << pTeamStatus->getPlace() << "\n";
      m_scoreboard["scoreboard"][team_id_]["points"] = pTeamStatus->getPoints();
      m_scoreboard["scoreboard"][team_id_]["place"] = pTeamStatus->getPlace();
      m_scoreboard["scoreboard"][team_id_]["tries"] = pTeamStatus->tries();
    }
  }
}

void scoreboard::update_services_statistics() {
  // std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
  // TODO update costs
  std::map<std::string, ctf01d::service_statistics *>::iterator it1;

  // nlohmann::json jsonCosts;
  for (it1 = m_service_costs_and_statistics.begin(); it1 != m_service_costs_and_statistics.end(); it1++) {
    std::string sId = it1->first;
    it1->second->updateJsonServiceStatistics(m_scoreboard["s_sta"][sId]);
  }
}

// std::string scoreboard::toString() {
//   std::lock_guard<std::mutex> lock(m_mutexFlagsLive);
//   std::string sResult = "";
//   std::map<std::string, ctf01d::team_status_row *>::iterator it;
//   for (it = m_teams_statuses.begin(); it != m_teams_statuses.end(); ++it){
//     sResult += it->first + ": \n"
//       "\tpoints: " + std::to_string(it->second->getPoints()) + "\n"
//       + it->second->servicesToString() + "\n";
//   }
//   return sResult;
// }

const nlohmann::json &scoreboard::to_json() {
  std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
  m_scoreboard["game"]["tc"] = WsjcppCore::getCurrentTimeInSeconds();
  return m_scoreboard;
}

} // namespace ctf01d
