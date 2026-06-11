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

#include "ctf01d_team_status_row.h"
#include "ctf01d_service_config.h"
#include <wsjcpp_employees.h>
#include "ctf01d/employees/employ_config.h"

namespace ctf01d {

team_status_row::team_status_row(
  const std::string &team_id,
  int nGameStartInSec,
  int nGameEndInSec
) {
  EmployConfig *pConfig = findWsjcppEmploy<EmployConfig>();
  const std::vector<ctf01d::service_config> &vServicesConf = pConfig->servicesConf();
  TAG = "team_status_row-" + team_id;
  m_sTeamId = team_id;
  m_place.store(0);
  m_nPoints = 0;

  for (unsigned int i = 0; i < vServicesConf.size(); i++) {
    ctf01d::service_config serviceConf = vServicesConf[i];
    std::string service_id = serviceConf.id();
    m_mapServicesStatus[service_id] = new Ctf01dServiceStatusCell(serviceConf.id());
  }
}

void team_status_row::setPlace(int val) {
  m_place.store(val);
}

int team_status_row::getPlace() {
  return m_place.load();
}

const std::string &team_status_row::teamId() {
  // std::lock_guard<std::mutex> lock(m_mutex);
  return m_sTeamId;
}

void team_status_row::setPoints(int nPoints) { // only for random
  std::lock_guard<std::mutex> lock(m_mutex);
  m_nPoints = nPoints;
}

int team_status_row::getPoints() {
  return m_nPoints;
}

void team_status_row::setServiceStatus(const std::string &service_id, std::string sStatus){
  // std::lock_guard<std::mutex> lock(m_mutex);
  m_mapServicesStatus[service_id]->setStatus(sStatus);
}

void team_status_row::setTries(int nTries) {
  m_nTries = nTries;
}

int team_status_row::tries() {
  return m_nTries;
}

std::string team_status_row::serviceStatus(const std::string &service_id){
  // std::lock_guard<std::mutex> lock(m_mutex);
  return m_mapServicesStatus[service_id]->status();
}

std::string team_status_row::servicesToString() {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::string sResult = "";
  /*std::map<int,std::string>::iterator it;
  for (it = m_mapServicesStatus.begin(); it != m_mapServicesStatus.end(); ++it){
    sResult += "\tservice" + std::to_string(it->first) + ": " + it->second + "\n";
  }*/
  return sResult;
}

void team_status_row::incrementDefense(const std::string &service_id, int nFlagPoints) {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_mapServicesStatus[service_id]->incrementDefenseFlags();
    m_mapServicesStatus[service_id]->addDefensePoints(nFlagPoints);
  }
  updatePoints();
}

int team_status_row::getDefenseFlags(const std::string &service_id) {
  return m_mapServicesStatus[service_id]->getDefenseFlags();
}

int team_status_row::getDefensePoints(const std::string &service_id) {
  return m_mapServicesStatus[service_id]->getDefensePoints();
}

void team_status_row::setServiceDefenseFlagsAndPoints(const std::string &service_id, int nDefenseFlags, int nDefensePoints) {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_mapServicesStatus[service_id]->setDefenseFlags(nDefenseFlags);
    m_mapServicesStatus[service_id]->setDefensePoints(nDefensePoints);
  }
  updatePoints();
}

void team_status_row::decrementFlagStollen(const std::string &service_id) {
  m_mapServicesStatus[service_id]->decrementFlagsStollen();
}

int team_status_row::getFlagsStollen(const std::string &service_id) {
  return m_mapServicesStatus[service_id]->getFlagsStollen();
}

void team_status_row::setFlagsStollen(const std::string &service_id, int val) {
  return m_mapServicesStatus[service_id]->setFlagsStollen(val);
}

void team_status_row::incrementAttack(const std::string &service_id, int nFlagPoints) {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_mapServicesStatus[service_id]->incrementAttackFlags();
    m_mapServicesStatus[service_id]->addAttackPoints(nFlagPoints);
  }
  updatePoints();
}

void team_status_row::setServiceAttackFlagsAndPoints(const std::string &service_id, int nAttackFlags, int nAttackPoints) {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_mapServicesStatus[service_id]->setAttackFlags(nAttackFlags);
    m_mapServicesStatus[service_id]->setAttackPoints(nAttackPoints);
  }
  updatePoints();
}

int team_status_row::getAttackFlags(const std::string &service_id) {
  return m_mapServicesStatus[service_id]->getAttackFlags();
}

int team_status_row::getAttackPoints(const std::string &service_id) {
  return m_mapServicesStatus[service_id]->getAttackPoints();
}

void team_status_row::updatePoints() {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_nPoints = 0;
  std::map<std::string, Ctf01dServiceStatusCell *>::iterator it;
  for (std::map<std::string, Ctf01dServiceStatusCell *>::iterator it = m_mapServicesStatus.begin(); it != m_mapServicesStatus.end(); ++it) {
    Ctf01dServiceStatusCell *pCell = it->second;
    int nSumAttackAndDefensePoints = pCell->getAttackPoints() + pCell->getDefensePoints();
    nSumAttackAndDefensePoints = nSumAttackAndDefensePoints * pCell->calculateSLA();
    // ctf01d::log::info(TAG, "nSumAttackAndDefensePoints 1 = " + std::to_string(nSumAttackAndDefensePoints));
    nSumAttackAndDefensePoints = nSumAttackAndDefensePoints / 100;
    // ctf01d::log::info(TAG, "nSumAttackAndDefensePoints 2 = " + std::to_string(nSumAttackAndDefensePoints));
    m_nPoints += nSumAttackAndDefensePoints;
  }
}

void team_status_row::setServiceFlagsForCalculateSLA(const std::string &service_id, int nPutsFlagsAllResults, int nPutsFlagsSuccessResults) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_mapServicesStatus[service_id]->setFlagsPutAllResultsCounter(nPutsFlagsAllResults);
  m_mapServicesStatus[service_id]->setFlagsPutSuccessResultsCounter(nPutsFlagsSuccessResults);
}

void team_status_row::incrementPutFlagSuccess(const std::string &service_id) {
  m_mapServicesStatus[service_id]->incrementPutFlagSuccess();
}

void team_status_row::incrementPutFlagFail(const std::string &service_id) {
  m_mapServicesStatus[service_id]->incrementPutFlagFail();
}

int team_status_row::calculateSLA(const std::string &service_id) {
  return m_mapServicesStatus[service_id]->calculateSLA();
}

} // namespace ctf01d
