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

#include "employ_scoreboard.h"
#include <wsjcpp_core.h>
#include <employ_config.h>
#include <cmath>
#include <stdio.h>
#include <string>
#include <map>
#include <mutex>
#include <vector>

// ---------------------------------------------------------------------
// Ctf01dServiceStatistics

Ctf01dServiceStatistics::Ctf01dServiceStatistics(const std::string &sServiceId) {
    TAG = "Ctf01dServiceStatistics-" + sServiceId;
    m_sServiceId = sServiceId;
    m_nAllStolenFlagsForService = 0;
    m_nAllDefenseFlagsForService = 0;
    m_sFirstBloodTeamId = "?";
}

int Ctf01dServiceStatistics::getAllStolenFlagsForService() {
    return m_nAllStolenFlagsForService;
}

void Ctf01dServiceStatistics::doIncrementStolenFlagsForService(int nAllStolenFlags) {
    m_nAllStolenFlagsForService++;
}

void Ctf01dServiceStatistics::setStolenFlagsForService(int nStolenFlags) {
    m_nAllStolenFlagsForService = nStolenFlags;
}

int Ctf01dServiceStatistics::getAllDefenseFlagsForService() {
    return m_nAllDefenseFlagsForService;
}

void Ctf01dServiceStatistics::doIncrementDefenseFlagsForService() {
    m_nAllDefenseFlagsForService++;
}

void Ctf01dServiceStatistics::setDefenseFlagsForService(int nAllDefenseFlagsForService) {
    m_nAllDefenseFlagsForService = nAllDefenseFlagsForService;
}

std::string Ctf01dServiceStatistics::getFirstBloodTeamId() {
    return m_sFirstBloodTeamId;
}

long Ctf01dServiceStatistics::getFirstBloodTime() {
    return m_nFirstBloodTimeInSeconds;
}

void Ctf01dServiceStatistics::updateJsonServiceStatistics(nlohmann::json &jsonCosts) {
    jsonCosts["af_att"] = m_nAllStolenFlagsForService;
    jsonCosts["af_def"] = m_nAllDefenseFlagsForService;
    jsonCosts["first_blood"] = m_sFirstBloodTeamId;
    jsonCosts["first_blood_ts"] = m_nFirstBloodTimeInSeconds;
}

void Ctf01dServiceStatistics::setFirstBloodTeamId(const std::string &sFirstBlood, long nDateACtion) {
    m_sFirstBloodTeamId = sFirstBlood;
    m_nFirstBloodTimeInSeconds = nDateACtion / 1000;
}

// ---------------------------------------------------------------------
// ServiceStatusCell


std::string ServiceStatusCell::SERVICE_UP = "up";
std::string ServiceStatusCell::SERVICE_DOWN = "down";
std::string ServiceStatusCell::SERVICE_MUMBLE = "mumble";
std::string ServiceStatusCell::SERVICE_CORRUPT = "corrupt";
std::string ServiceStatusCell::SERVICE_SHIT = "shit";
std::string ServiceStatusCell::SERVICE_WAIT = "wait";
std::string ServiceStatusCell::SERVICE_COFFEEBREAK = "coffeebreak";

ServiceStatusCell::ServiceStatusCell(const std::string &sServiceId) {
    m_nUpPointTimeInSec = WsjcppCore::getCurrentTimeInSeconds();
    TAG = "ServiceStatusCell-" + sServiceId;
    m_sServiceId = sServiceId;
    m_sStatus = ServiceStatusCell::SERVICE_DOWN;
    m_nDefenseFlags = 0;
    m_nAttackFlags = 0;
    m_nAttackPoints = 0;
    m_nDefensePoints = 0;

}

const std::string &ServiceStatusCell::serviceId() {
    return m_sServiceId;
}

void ServiceStatusCell::setDefenseFlags(int nDefenseFlags) {
    std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
    m_nDefenseFlags = nDefenseFlags;
}

int ServiceStatusCell::getDefenseFlags() {
    return m_nDefenseFlags;
}

void ServiceStatusCell::incrementDefenseFlags() {
    std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
    m_nDefenseFlags++;
}

void ServiceStatusCell::setDefensePoints(int nDefensePoints) {
    std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
    m_nDefensePoints = nDefensePoints;
}

int ServiceStatusCell::getDefensePoints() {
    return m_nDefensePoints;
}

void ServiceStatusCell::addDefensePoints(int nDefensePoints) {
    std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
    m_nDefensePoints += nDefensePoints;
}

void ServiceStatusCell::setAttackFlags(int nAttackFlags) {
    std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
    m_nAttackFlags = nAttackFlags;
}

int ServiceStatusCell::getAttackFlags() {
    return m_nAttackFlags;
}

void ServiceStatusCell::incrementAttackFlags() {
    std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
    m_nAttackFlags++;
}

void ServiceStatusCell::setAttackPoints(int nAttackPoints) {
    std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
    m_nAttackPoints = nAttackPoints;
}

int ServiceStatusCell::getAttackPoints() {
    return m_nAttackPoints;
}

void ServiceStatusCell::addAttackPoints(int nAttackPoints) {
    std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
    m_nAttackPoints += nAttackPoints;
}

void ServiceStatusCell::setFlagsPutAllResultsCounter(int nFlagsPutAllResultsCounter) {
    m_nFlagsPutAllResultsCounter = nFlagsPutAllResultsCounter;
}

void ServiceStatusCell::setFlagsPutSuccessResultsCounter(int nFlagsPutSuccessResultsCounter) {
    m_nFlagsPutSuccessResultsCounter = nFlagsPutSuccessResultsCounter;
}

void ServiceStatusCell::incrementPutFlagSuccess() {
    m_nFlagsPutSuccessResultsCounter++;
    m_nFlagsPutAllResultsCounter++;
}

void ServiceStatusCell::incrementPutFlagFail() {
    m_nFlagsPutAllResultsCounter++;
}

int ServiceStatusCell::calculateSLA() {
    if (m_nFlagsPutAllResultsCounter == 0) {
        if (m_nFlagsPutSuccessResultsCounter != 0) {
            // WsjcppLog::warn(TAG, "Could not possible situation!");
        }
        return 100;
    }
    return (m_nFlagsPutSuccessResultsCounter*100) / m_nFlagsPutAllResultsCounter;
}

void ServiceStatusCell::setStatus(const std::string &sStatus) {
    std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
    m_sStatus = sStatus;
    if (sStatus != ServiceStatusCell::SERVICE_UP) {
        m_nUpPointTimeInSec = WsjcppCore::getCurrentTimeInSeconds();
    }
}

std::string ServiceStatusCell::status() {
    return m_sStatus;
}

// ----------------------------------------------------------------------
// TeamStatusRow

TeamStatusRow::TeamStatusRow(
    const std::string &sTeamId,
    int nGameStartInSec,
    int nGameEndInSec
) {
    EmployConfig *pConfig = findWsjcppEmploy<EmployConfig>();
    const std::vector<Ctf01dServiceDef> &vServicesConf = pConfig->servicesConf();
    TAG = "TeamStatusRow-" + sTeamId;
    m_sTeamId = sTeamId;
    m_nPlace = 0;
    m_nPoints = 0;

    for (unsigned int i = 0; i < vServicesConf.size(); i++) {
        Ctf01dServiceDef serviceConf = vServicesConf[i];
        std::string sServiceId = serviceConf.id();
        m_mapServicesStatus[sServiceId] = new ServiceStatusCell(serviceConf.id());
    }
}

void TeamStatusRow::setPlace(int nPlace) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_nPlace = nPlace;
}

int TeamStatusRow::getPlace() {
    // std::lock_guard<std::mutex> lock(m_mutex);
    return m_nPlace;
}

const std::string &TeamStatusRow::teamId() {
    // std::lock_guard<std::mutex> lock(m_mutex);
    return m_sTeamId;
}

void TeamStatusRow::setPoints(int nPoints) { // only for random
    std::lock_guard<std::mutex> lock(m_mutex);
    m_nPoints = nPoints;
}

int TeamStatusRow::getPoints() {
    return m_nPoints;
}

void TeamStatusRow::setServiceStatus(const std::string &sServiceId, std::string sStatus){
    // std::lock_guard<std::mutex> lock(m_mutex);
    m_mapServicesStatus[sServiceId]->setStatus(sStatus);
}

void TeamStatusRow::setTries(int nTries) {
    m_nTries = nTries;
}

int TeamStatusRow::tries() {
    return m_nTries;
}

std::string TeamStatusRow::serviceStatus(const std::string &sServiceId){
    // std::lock_guard<std::mutex> lock(m_mutex);
    return m_mapServicesStatus[sServiceId]->status();
}

std::string TeamStatusRow::servicesToString() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string sResult = "";
    /*std::map<int,std::string>::iterator it;
    for (it = m_mapServicesStatus.begin(); it != m_mapServicesStatus.end(); ++it){
        sResult += "\tservice" + std::to_string(it->first) + ": " + it->second + "\n";
    }*/
    return sResult;
}

void TeamStatusRow::incrementDefense(const std::string &sServiceId, int nFlagPoints) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_mapServicesStatus[sServiceId]->incrementDefenseFlags();
        m_mapServicesStatus[sServiceId]->addDefensePoints(nFlagPoints);
    }
    updatePoints();
}

int TeamStatusRow::getDefenseFlags(const std::string &sServiceId) {
    return m_mapServicesStatus[sServiceId]->getDefenseFlags();
}

int TeamStatusRow::getDefensePoints(const std::string &sServiceId) {
    return m_mapServicesStatus[sServiceId]->getDefensePoints();
}

void TeamStatusRow::setServiceDefenseFlagsAndPoints(const std::string &sServiceId, int nDefenseFlags, int nDefensePoints) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_mapServicesStatus[sServiceId]->setDefenseFlags(nDefenseFlags);
        m_mapServicesStatus[sServiceId]->setDefensePoints(nDefensePoints);
    }
    updatePoints();
}

void TeamStatusRow::incrementAttack(const std::string &sServiceId, int nFlagPoints) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_mapServicesStatus[sServiceId]->incrementAttackFlags();
        m_mapServicesStatus[sServiceId]->addAttackPoints(nFlagPoints);
    }
    updatePoints();
}

void TeamStatusRow::setServiceAttackFlagsAndPoints(const std::string &sServiceId, int nAttackFlags, int nAttackPoints) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_mapServicesStatus[sServiceId]->setAttackFlags(nAttackFlags);
        m_mapServicesStatus[sServiceId]->setAttackPoints(nAttackPoints);
    }
    updatePoints();
}

int TeamStatusRow::getAttackFlags(const std::string &sServiceId) {
    return m_mapServicesStatus[sServiceId]->getAttackFlags();
}

int TeamStatusRow::getAttackPoints(const std::string &sServiceId) {
    return m_mapServicesStatus[sServiceId]->getAttackPoints();
}

void TeamStatusRow::updatePoints() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_nPoints = 0;
    std::map<std::string, ServiceStatusCell *>::iterator it;
    for (std::map<std::string, ServiceStatusCell *>::iterator it = m_mapServicesStatus.begin(); it != m_mapServicesStatus.end(); ++it) {
        ServiceStatusCell *pCell = it->second;
        int nSumAttackAndDefensePoints = pCell->getAttackPoints() + pCell->getDefensePoints();
        nSumAttackAndDefensePoints = nSumAttackAndDefensePoints * pCell->calculateSLA();
        // WsjcppLog::info(TAG, "nSumAttackAndDefensePoints 1 = " + std::to_string(nSumAttackAndDefensePoints));
        nSumAttackAndDefensePoints = nSumAttackAndDefensePoints / 100;
        // WsjcppLog::info(TAG, "nSumAttackAndDefensePoints 2 = " + std::to_string(nSumAttackAndDefensePoints));
        m_nPoints += nSumAttackAndDefensePoints;
    }
}

void TeamStatusRow::setServiceFlagsForCalculateSLA(const std::string &sServiceId, int nPutsFlagsAllResults, int nPutsFlagsSuccessResults) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_mapServicesStatus[sServiceId]->setFlagsPutAllResultsCounter(nPutsFlagsAllResults);
    m_mapServicesStatus[sServiceId]->setFlagsPutSuccessResultsCounter(nPutsFlagsSuccessResults);
}

void TeamStatusRow::incrementPutFlagSuccess(const std::string &sServiceId) {
    m_mapServicesStatus[sServiceId]->incrementPutFlagSuccess();
}

void TeamStatusRow::incrementPutFlagFail(const std::string &sServiceId) {
    m_mapServicesStatus[sServiceId]->incrementPutFlagFail();
}

int TeamStatusRow::calculateSLA(const std::string &sServiceId) {
    return m_mapServicesStatus[sServiceId]->calculateSLA();
}

// ---------------------------------------------------------------------
// EmployScoreboard

REGISTRY_WJSCPP_SERVICE_LOCATOR(EmployScoreboard)

EmployScoreboard::EmployScoreboard()
: WsjcppEmployBase(EmployScoreboard::name(), {}) {
    TAG = EmployScoreboard::name();
}

// ---------------------------------------------------------------------

bool EmployScoreboard::init() {
    EmployConfig *pEmployConfig = findWsjcppEmploy<EmployConfig>();
    const std::vector<Ctf01dTeamDef> &vTeamsConf = pEmployConfig->teamsConf();
    const std::vector<Ctf01dServiceDef> &vServicesConf = pEmployConfig->servicesConf();

    // keep the list of the services ids
    for (unsigned int i = 0; i < vServicesConf.size(); i++) {
        std::string sServiceId = vServicesConf[i].id();
        m_mapServiceCostsAndStatistics[sServiceId] = new Ctf01dServiceStatistics(sServiceId);
    }

    return true;
}

// ---------------------------------------------------------------------

bool EmployScoreboard::deinit() {
    WsjcppLog::info(TAG, "deinit");
    return true;
}


