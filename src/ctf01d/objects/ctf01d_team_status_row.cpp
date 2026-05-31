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
#include "ctf01d_service_def.h"
#include <wsjcpp_employees.h>
#include "ctf01d/employees/employ_config.h"

Ctf01dTeamStatusRow::Ctf01dTeamStatusRow(
  const std::string &team_id,
  int nGameStartInSec,
  int nGameEndInSec
) {
  EmployConfig *pConfig = findWsjcppEmploy<EmployConfig>();
  const std::vector<ctf01d::service_def> &vServicesConf = pConfig->servicesConf();
  TAG = "Ctf01dTeamStatusRow-" + team_id;
  m_sTeamId = team_id;
  m_nPlace = 0;
  m_nPoints = 0;

  for (unsigned int i = 0; i < vServicesConf.size(); i++) {
    ctf01d::service_def serviceConf = vServicesConf[i];
    std::string service_id = serviceConf.id();
    m_mapServicesStatus[service_id] = new Ctf01dServiceStatusCell(serviceConf.id());
  }
}

void Ctf01dTeamStatusRow::setPlace(int nPlace) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_nPlace = nPlace;
}

int Ctf01dTeamStatusRow::getPlace() {
  // std::lock_guard<std::mutex> lock(m_mutex);
  return m_nPlace;
}

const std::string &Ctf01dTeamStatusRow::teamId() {
  // std::lock_guard<std::mutex> lock(m_mutex);
  return m_sTeamId;
}

void Ctf01dTeamStatusRow::setPoints(int nPoints) { // only for random
  std::lock_guard<std::mutex> lock(m_mutex);
  m_nPoints = nPoints;
}

int Ctf01dTeamStatusRow::getPoints() {
  return m_nPoints;
}

void Ctf01dTeamStatusRow::setServiceStatus(const std::string &service_id, std::string sStatus){
  // std::lock_guard<std::mutex> lock(m_mutex);
  m_mapServicesStatus[service_id]->setStatus(sStatus);
}

void Ctf01dTeamStatusRow::setTries(int nTries) {
  m_nTries = nTries;
}

int Ctf01dTeamStatusRow::tries() {
  return m_nTries;
}

std::string Ctf01dTeamStatusRow::serviceStatus(const std::string &service_id){
  // std::lock_guard<std::mutex> lock(m_mutex);
  return m_mapServicesStatus[service_id]->status();
}

std::string Ctf01dTeamStatusRow::servicesToString() {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::string sResult = "";
  /*std::map<int,std::string>::iterator it;
  for (it = m_mapServicesStatus.begin(); it != m_mapServicesStatus.end(); ++it){
    sResult += "\tservice" + std::to_string(it->first) + ": " + it->second + "\n";
  }*/
  return sResult;
}

void Ctf01dTeamStatusRow::incrementDefense(const std::string &service_id, int nFlagPoints) {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_mapServicesStatus[service_id]->incrementDefenseFlags();
    m_mapServicesStatus[service_id]->addDefensePoints(nFlagPoints);
  }
  updatePoints();
}

int Ctf01dTeamStatusRow::getDefenseFlags(const std::string &service_id) {
  return m_mapServicesStatus[service_id]->getDefenseFlags();
}

int Ctf01dTeamStatusRow::getDefensePoints(const std::string &service_id) {
  return m_mapServicesStatus[service_id]->getDefensePoints();
}

void Ctf01dTeamStatusRow::setServiceDefenseFlagsAndPoints(const std::string &service_id, int nDefenseFlags, int nDefensePoints) {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_mapServicesStatus[service_id]->setDefenseFlags(nDefenseFlags);
    m_mapServicesStatus[service_id]->setDefensePoints(nDefensePoints);
  }
  updatePoints();
}

void Ctf01dTeamStatusRow::incrementAttack(const std::string &service_id, int nFlagPoints) {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_mapServicesStatus[service_id]->incrementAttackFlags();
    m_mapServicesStatus[service_id]->addAttackPoints(nFlagPoints);
  }
  updatePoints();
}

void Ctf01dTeamStatusRow::setServiceAttackFlagsAndPoints(const std::string &service_id, int nAttackFlags, int nAttackPoints) {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_mapServicesStatus[service_id]->setAttackFlags(nAttackFlags);
    m_mapServicesStatus[service_id]->setAttackPoints(nAttackPoints);
  }
  updatePoints();
}

int Ctf01dTeamStatusRow::getAttackFlags(const std::string &service_id) {
  return m_mapServicesStatus[service_id]->getAttackFlags();
}

int Ctf01dTeamStatusRow::getAttackPoints(const std::string &service_id) {
  return m_mapServicesStatus[service_id]->getAttackPoints();
}

void Ctf01dTeamStatusRow::updatePoints() {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_nPoints = 0;
  std::map<std::string, Ctf01dServiceStatusCell *>::iterator it;
  for (std::map<std::string, Ctf01dServiceStatusCell *>::iterator it = m_mapServicesStatus.begin(); it != m_mapServicesStatus.end(); ++it) {
    Ctf01dServiceStatusCell *pCell = it->second;
    int nSumAttackAndDefensePoints = pCell->getAttackPoints() + pCell->getDefensePoints();
    nSumAttackAndDefensePoints = nSumAttackAndDefensePoints * pCell->calculateSLA();
    // WsjcppLog::info(TAG, "nSumAttackAndDefensePoints 1 = " + std::to_string(nSumAttackAndDefensePoints));
    nSumAttackAndDefensePoints = nSumAttackAndDefensePoints / 100;
    // WsjcppLog::info(TAG, "nSumAttackAndDefensePoints 2 = " + std::to_string(nSumAttackAndDefensePoints));
    m_nPoints += nSumAttackAndDefensePoints;
  }
}

void Ctf01dTeamStatusRow::setServiceFlagsForCalculateSLA(const std::string &service_id, int nPutsFlagsAllResults, int nPutsFlagsSuccessResults) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_mapServicesStatus[service_id]->setFlagsPutAllResultsCounter(nPutsFlagsAllResults);
  m_mapServicesStatus[service_id]->setFlagsPutSuccessResultsCounter(nPutsFlagsSuccessResults);
}

void Ctf01dTeamStatusRow::incrementPutFlagSuccess(const std::string &service_id) {
  m_mapServicesStatus[service_id]->incrementPutFlagSuccess();
}

void Ctf01dTeamStatusRow::incrementPutFlagFail(const std::string &service_id) {
  m_mapServicesStatus[service_id]->incrementPutFlagFail();
}

int Ctf01dTeamStatusRow::calculateSLA(const std::string &service_id) {
  return m_mapServicesStatus[service_id]->calculateSLA();
}