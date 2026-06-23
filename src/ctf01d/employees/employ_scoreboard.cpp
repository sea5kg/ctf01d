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

#include <wsjcpp_core.h>
#include <wsjcpp_employees.h>
#include <json.hpp>
#include <sea5kg_logger.h>
#include <cmath>
#include <stdio.h>
#include <map>
#include <mutex>
#include <vector>
#include "ctf01d/include/ctf01d_config.h"
#include "ctf01d/include/ctf01d_alive_flags.h"
#include "ctf01d/include/ctf01d_globals.h"
#include "ctf01d/include/ctf01d_scoreboard.h"
#include "ctf01d/objects/ctf01d_service_statistics.h"
#include "ctf01d/objects/ctf01d_team_status_row.h"
#include "ctf01d/objects/ctf01d_formulas_for_points.h"
#include "ctf01d/objects/ctf01d_var.h"
#include "ctf01d/include/ctf01d_activities.h"
#include "ctf01d/include/ctf01d_database.h"

class employ_scoreboard : public WsjcppEmployBase, public ctf01d::scoreboard {
public:
  employ_scoreboard();
  virtual bool init(const std::string &sName, bool bSilent) override;
  virtual bool deinit(const std::string &sName, bool bSilent) override;

  virtual void set_service_status(const std::string &team_id, const std::string &service_id, const std::string &status) override;
  virtual void insert_flag_attempt(const std::string &thief_team_id, const std::string &flag_value, const std::string &request_ip) override;
  virtual void init_state_from_storage() override;
  // Returns flag points on success; std::nullopt if this team has already
  // stolen the flag (dedup check happens under the same lock as the insert
  // so concurrent submissions can't double-credit).
  virtual std::optional<int> increment_attack_score(const ctf01d::flag &flag, const std::string &team_id) override;
  virtual void increment_defense_score(const ctf01d::flag &flag) override;
  virtual void increment_flags_putted_and_service_up(const ctf01d::flag &flag) override;
  virtual void insert_flag_put_fail(const ctf01d::flag &flag, const std::string &service_status, const std::string &description_status) override;
  virtual std::string service_status(const std::string &team_id, const std::string &service_id) override;
  virtual const nlohmann::json &to_json() override;

private:
  bool init_services_stats();

  std::string TAG;
  ctf01d::alive_flags *m_alive_flags;
  ctf01d::activities *m_activities;
  ctf01d::database *m_database;
  std::shared_ptr<ctf01d::var_int> m_flag_cost_in_points;
  int m_game_start_in_seconds;
  int m_game_end_in_seconds;
  int m_game_coffee_break_start_in_seconds;
  int m_game_coffee_break_end_in_seconds;
  int m_team_count;

  void sort_places(); // TODO merge this function with update costs
  void update_services_statistics();

  // TODO move to employ scoreboard
  std::map<std::string, ctf01d::service_statistics *> m_service_statistics;
  int m_all_defense_flags;

  std::string random_service_status();
  bool m_random;
  // TODO shared ptr and move to employ scoreboard
  std::map<std::string, ctf01d::team_status_row *> m_teams_statuses;

  std::mutex m_mutex_scoreboard;
  nlohmann::json m_scoreboard; // prepare data for scoreboard
  void init_json_scoreboard();
  void update_json_scoreboard();

  std::shared_ptr<ctf01d::formulas_for_points> m_formulas;

  // std::string TAG;
  // std::shared_ptr<ctf01d::scoreboard> m_scoreboard;
  // std::mutex m_mutex_services_statistics;
  // std::map<std::string, std::shared_ptr<ctf01d::service_statistics>> m_services_statistics;
};


REGISTRY_WSJCPP_EMPLOY(employ_scoreboard)

employ_scoreboard::employ_scoreboard()
: WsjcppEmployBase({ ctf01d::scoreboard::name() }, { ctf01d::config::name(), ctf01d::database::name(), ctf01d::activities::name() }) {
  TAG = employ_scoreboard::name();
}

bool employ_scoreboard::init(const std::string &sName, bool bSilent) {
  if (!init_services_stats()) {
    return false;
  }

  // scoreboard
  auto config = findWsjcppEmploy<ctf01d::config>();
  m_database = findWsjcppEmploy<ctf01d::database>();
  const std::vector<ctf01d::team_config> &vTeamsConf = config->teams();
  const std::vector<ctf01d::service_config> &vServicesConf = config->services();
  m_random = config->scoreboard_random();
  std::string scoreboard_random = "Scoreboard random: ";
  scoreboard_random = scoreboard_random + (m_random ? "yes" : "no");
  sea5kg::log::warn(TAG, scoreboard_random);
  std::srand(unsigned(std::time(0)));
  m_game_start_in_seconds = config->game_start_utc_in_seconds();
  m_game_end_in_seconds = config->game_end_utc_in_seconds();
  m_game_coffee_break_start_in_seconds = config->game_coffee_break_start_utc_in_seconds();
  m_game_coffee_break_end_in_seconds = config->game_coffee_break_end_utc_in_seconds();
  m_all_defense_flags = 0;
  m_flag_cost_in_points = config->get_flag_cost_in_points();
  m_team_count = vTeamsConf.size();
  m_alive_flags = findWsjcppEmploy<ctf01d::alive_flags>();
  m_activities = findWsjcppEmploy<ctf01d::activities>();
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
        // m_teams_statuses[team_id]->setTries(std::rand() % 1000);
      }
    }
  }

  // keep the list of the services ids
  for (unsigned int i = 0; i < vServicesConf.size(); i++) {
    std::string service_id = vServicesConf[i].id();
    m_service_statistics[service_id] = new ctf01d::service_statistics(service_id);
  }

  init_json_scoreboard();

  sea5kg::log::info(TAG, "Restoring states from storage...");
  init_state_from_storage();
  sea5kg::log::ok(TAG, "Restored state from storage.");

  return true;
}

bool employ_scoreboard::deinit(const std::string &sName, bool bSilent) {
  sea5kg::log::info(TAG, "deinit");
  return true;
}


void employ_scoreboard::init_json_scoreboard() {
  std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
  m_scoreboard.clear();
  m_scoreboard["s_sta"] = nlohmann::json();

  auto config = findWsjcppEmploy<ctf01d::config>();
  const std::vector<ctf01d::team_config> &vTeamsConf = config->teams();
  const std::vector<ctf01d::service_config> &vServices = config->services();

  nlohmann::json jsonServicesStatistics;
  for (unsigned int iservice = 0; iservice < vServices.size(); iservice++) {
    ctf01d::service_config serviceConf = vServices[iservice];
    m_scoreboard["s_sta"][serviceConf.id()] = nlohmann::json();
    m_service_statistics[serviceConf.id()]->update_scoreboard(m_scoreboard);
  }
  m_scoreboard[ctf01d::json_fields::SUMMARY_ACTIVITIES] = 0;

  nlohmann::json jsonScoreboard;
  for (unsigned int i_team = 0; i_team < vTeamsConf.size(); ++i_team) {
    ctf01d::team_config teamConf = vTeamsConf[i_team];
    std::string team_id = teamConf.id();
    nlohmann::json teamData;
    teamData["place"] = m_teams_statuses[team_id]->getPlace();
    teamData["points"] = m_teams_statuses[team_id]->getPoints();
    teamData[ctf01d::json_fields::TRIES] = 0;
    teamData[ctf01d::json_fields::UPDATED] = 0;
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
  jsonGame["t0"] = config->game_start_utc_in_seconds();
  jsonGame["t1"] = config->game_coffee_break_start_utc_in_seconds();
  jsonGame["t2"] = config->game_coffee_break_end_utc_in_seconds();
  jsonGame["t3"] = config->game_end_utc_in_seconds();
  jsonGame["tc"] = WsjcppCore::getCurrentTimeInSeconds();
  m_scoreboard["game"] = jsonGame;
}

void employ_scoreboard::update_json_scoreboard() {
  std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
  // TODO update score
  // TODO update costs
}

bool employ_scoreboard::init_services_stats() {
  // std::lock_guard<std::mutex> lock(m_mutex_services_statistics);
  // m_services_statistics.clear();

  // auto config = findWsjcppEmploy<ctf01d::config>();
  // const std::vector<ctf01d::team_config> &teams_conf = config->teams();
  // const std::vector<ctf01d::service_config> &services_conf = config->services();

  // // keep the list of the services ids
  // for (unsigned int i = 0; i < services_conf.size(); i++) {
  //   std::string service_id = services_conf[i].id();
  //   m_services_statistics[service_id] = std::make_shared<ctf01d::service_statistics>(service_id);
  // }
  return true;
}

std::string employ_scoreboard::random_service_status() {
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

void employ_scoreboard::set_service_status(const std::string &team_id, const std::string &service_id, const std::string &status) {
  std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
  std::string sNewStatus = m_random ? random_service_status() : status;

  std::map<std::string, ctf01d::team_status_row *>::iterator it;
  it = m_teams_statuses.find(team_id);
  if (it != m_teams_statuses.end()) {
    if (it->second->serviceStatus(service_id) != sNewStatus) {
      it->second->setServiceStatus(service_id, sNewStatus);
      m_scoreboard["scoreboard"][team_id]["ts_sta"][service_id]["status"] = sNewStatus;
    }
  }
}

void employ_scoreboard::insert_flag_attempt(const std::string &thief_team_id, const std::string &flag_value, const std::string &request_ip) {
  std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
  m_activities->insert_flag_attempt(thief_team_id, flag_value, request_ip, m_scoreboard);
}

void employ_scoreboard::init_state_from_storage() {
  auto config = findWsjcppEmploy<ctf01d::config>();
  const std::vector<ctf01d::service_config> &vServices = config->services();

  // load services statistics
  sea5kg::log::info(TAG, "Loading services statistics...");
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
    f.nStolenFlags = m_database->number_of_stolen_flags_for_service(sServiceID);
    f.nDefenseFlags = m_database->number_of_defense_flag_for_service(sServiceID);
    if (f.nStolenFlags > 0) {
      std::pair<std::string, long> fb = m_database->get_first_blood_from_stolen_flags_for_service(sServiceID);
      f.sFirstBloodTeamID = fb.first;
      f.nFirstBloodTime = fb.second;
    }
    m_all_defense_flags += f.nDefenseFlags;
    vFlags.push_back(f);
  }

  sea5kg::log::info(TAG, "Setting services statistics...");
  for (int i = 0; i < vFlags.size(); i++) {
    FlagsForService f = vFlags[i];
    m_service_statistics[f.sServiceID]->set_flags_stolen(f.nStolenFlags);
    m_service_statistics[f.sServiceID]->set_flags_defense(f.nDefenseFlags);
    if (f.nStolenFlags > 0) {
      m_service_statistics[f.sServiceID]->set_first_blood_team_id(f.sFirstBloodTeamID, f.nFirstBloodTime);
    }
  }

  sea5kg::log::info(TAG, "Setting teams statistics...");
  std::map<std::string, ctf01d::team_status_row *>::iterator it;
  for (it = m_teams_statuses.begin(); it != m_teams_statuses.end(); it++) {
    ctf01d::team_status_row *pRow = it->second;

    m_scoreboard["scoreboard"][pRow->teamId()][ctf01d::json_fields::TRIES] = 0;

    for (unsigned int i = 0; i < vServices.size(); i++) {
      std::string sServiceID = vServices[i].id();

      // calculate defense
      sea5kg::log::info(TAG, "   -> (" + pRow->teamId() + ") calculate defense");
      int nDefenseFlags = m_database->number_of_flags_defense(pRow->teamId(), sServiceID);
      int nDefensePoints = m_database->sum_points_of_flags_defense(pRow->teamId(), sServiceID);
      pRow->setServiceDefenseFlagsAndPoints(sServiceID, nDefenseFlags, nDefensePoints);
      m_scoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["def"] = nDefenseFlags;
      m_scoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["pt_def"] = nDefensePoints;

      // calculate attack
      sea5kg::log::info(TAG, "   -> (" + pRow->teamId() + ") calculate attack and flags stollen");
      int nAttackFlags = m_database->number_of_flags_stollen(pRow->teamId(), sServiceID);
      int nFlagsStollen = -1 * m_database->number_of_flags_stollen_by_victim(pRow->teamId(), sServiceID);
      int nAttackPoints = m_database->sum_points_of_flags_stolen(pRow->teamId(), sServiceID);
      pRow->setServiceAttackFlagsAndPoints(sServiceID, nAttackFlags, nAttackPoints);
      pRow->setFlagsStollen(sServiceID, nFlagsStollen);
      m_scoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["att_st"] = nFlagsStollen;
      m_scoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["att"] = nAttackFlags;
      m_scoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["pt_att"] = nAttackPoints;

      // calculate uptime / sla
      sea5kg::log::info(TAG, "   -> (" + pRow->teamId() + ") uptime / sla");
      int nPutsFlagsAllResults = m_database->number_of_flags_checker_put_all_results(pRow->teamId(), sServiceID);
      int nPutsFlagsSuccessResults = m_database->number_of_flags_checker_put_success_result(pRow->teamId(), sServiceID);
      pRow->setServiceFlagsForCalculateSLA(sServiceID, nPutsFlagsAllResults, nPutsFlagsSuccessResults);
      m_scoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["sla"] = pRow->calculateSLA(sServiceID);
    }
  }

  sea5kg::log::info(TAG, "Sorting places and apply to json...");
  {
    std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
    sort_places();
    update_services_statistics();
  }
  sea5kg::log::info(TAG, "Updating activities...");
  m_activities->update_scoreboard(m_scoreboard);
}

std::optional<int> employ_scoreboard::increment_attack_score(const ctf01d::flag &flag, const std::string &team_id) {
  std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
  if (m_database->is_already_stole(flag, team_id)) {
    return std::nullopt;
  }
  std::string service_id = flag.service_id();

  // TODO calculate
  // int nFlagPoints = m_service_statistics[service_id]->getCostStolenFlag()*10; // one number after dot
  int flag_points = m_flag_cost_in_points->value(); // TODO basic
  long date_action = WsjcppCore::getCurrentTimeInMilliseconds();
  // victim place in scoreboard
  std::map<std::string, ctf01d::team_status_row *>::iterator it_victim;
  it_victim = m_teams_statuses.find(flag.team_id());
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
    m_database->insert_to_flags_stolen(flag, team_id, flag_points, date_action, victim_place, thief_place);
    pRow->incrementAttack(service_id, flag_points);
    pRow->updatePoints();
    if (row_victim != nullptr) {
      row_victim->decrementFlagStollen(service_id);
      m_scoreboard["scoreboard"][flag.team_id()]["ts_sta"][service_id]["att_st"] = row_victim->getFlagsStollen(service_id);
    }
    m_scoreboard["scoreboard"][team_id]["ts_sta"][service_id]["att"] = pRow->getAttackFlags(service_id);
    m_scoreboard["scoreboard"][team_id]["ts_sta"][service_id]["pt_att"] = pRow->getAttackPoints(service_id);
    m_scoreboard["scoreboard"][team_id]["ts_sta"][service_id]["sla"] = pRow->calculateSLA(service_id);
    m_scoreboard["scoreboard"][team_id]["points"] = pRow->getPoints();
    sort_places();
  }

  std::map<std::string, ctf01d::service_statistics *>::iterator it2;
  it2 = m_service_statistics.find(service_id);
  if (it2 != m_service_statistics.end()) {
    it2->second->do_increment_flags_stolen();
    if (it2->second->first_blood_team_id() == "?") {
      it2->second->set_first_blood_team_id(team_id, date_action);
    }
    update_services_statistics();
  }
  return std::optional<int>(flag_points);
}

void employ_scoreboard::increment_defense_score(const ctf01d::flag &flag) {
  std::lock_guard<std::mutex> lock(m_mutex_scoreboard);

  std::string team_id = flag.team_id();
  std::string service_id = flag.service_id();
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
  it2 = m_service_statistics.find(service_id);
  if (it2 != m_service_statistics.end()) {
    m_all_defense_flags++;
    it2->second->do_increment_flags_defense();
    update_services_statistics();
  }
}

void employ_scoreboard::increment_flags_putted_and_service_up(const ctf01d::flag &flag) {
  std::string service_id = flag.service_id();
  std::string team_id = flag.team_id();
  std::string sNewStatus = m_random ? random_service_status() : ctf01d::service_status_cell::SERVICE_UP;

  if (m_alive_flags->insert_alive_flag(flag)) {
    // m_database->insertToFlagLive(flag);
    m_database->insert_to_flags_checker_put_result(flag, "up");
    m_teams_statuses[flag.team_id()]->incrementPutFlagSuccess(flag.service_id());
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

void employ_scoreboard::insert_flag_put_fail(const ctf01d::flag &flag, const std::string &service_status, const std::string &description_status) {
  m_database->insert_to_flags_checker_put_result(flag, description_status);

  std::lock_guard<std::mutex> lock(m_mutex_scoreboard);

  std::string service_id = flag.service_id();
  std::string team_id = flag.team_id();
  std::string sNewStatus = m_random ? random_service_status() : service_status;

  std::map<std::string, ctf01d::team_status_row *>::iterator it;
  it = m_teams_statuses.find(flag.team_id());
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

std::string employ_scoreboard::service_status(const std::string &team_id, const std::string &service_id) {
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

void employ_scoreboard::sort_places() {
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
      // m_scoreboard["scoreboard"][team_id_]["tries"] = pTeamStatus->tries();
    }
  }
}

void employ_scoreboard::update_services_statistics() {
  // std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
  // TODO update costs
  std::map<std::string, ctf01d::service_statistics *>::iterator it1;

  // nlohmann::json jsonCosts;
  for (it1 = m_service_statistics.begin(); it1 != m_service_statistics.end(); it1++) {
    std::string sId = it1->first;
    it1->second->update_scoreboard(m_scoreboard);
  }
}

const nlohmann::json &employ_scoreboard::to_json() {
  std::lock_guard<std::mutex> lock(m_mutex_scoreboard);
  m_scoreboard["game"]["tc"] = WsjcppCore::getCurrentTimeInSeconds();
  return m_scoreboard;
}
