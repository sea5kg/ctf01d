/**********************************************************************************
 *          Project
 *  _______ _________ _______  _______  __    ______
 * (  ____ \\__   __/(  ____ \(  __   )/  \  (  __  \
 * | (    \/   ) (   | (    \/| (  )  |\/) ) | (  \  )
 * | |         | |   | (__    | | /   |  | | | |   ) |
 * | |         | |   |  __)   | (/ /) |  | | | |   | |
 * | |         | |   | (      |   / | |  | | | |   ) |
 * | (____/\   | |   | )      |  (__) |__) (_| (__/  )
 * (_______/   )_(   |/       (_______)\____/(______/
 *
 * MIT License
 * Copyright (c) 2018-2025 Evgenii Sopov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 ***********************************************************************************/

#include "ctf01d_scoreboard.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <wsjcpp_core.h>
#include <employ_config.h>
#include <employ_flags.h>

// ---------------------------------------------------------------------
// Scoreboard

Ctf01dScoreboard::Ctf01dScoreboard(
    bool bRandom,
    int nGameStartInSec,
    int nGameEndInSec,
    int nGameCoffeeBreakStartInSec,
    int nGameCoffeeBreakEndInSec,
    int nFlagTimeLiveInSec
) {
    EmployConfig *pEmployConfig = findWsjcppEmploy<EmployConfig>();
    m_pDatabase = findWsjcppEmploy<EmployDatabase>();
    const std::vector<Ctf01dTeamDef> &vTeamsConf = pEmployConfig->teamsConf();
    const std::vector<Ctf01dServiceDef> &vServicesConf = pEmployConfig->servicesConf();

    TAG = "Ctf01dScoreboard";
    m_bRandom = bRandom;
    std::string sScroreboardRandom = "Scoreboard random: ";
    sScroreboardRandom = sScroreboardRandom + (m_bRandom ? "yes" : "no");
    WsjcppLog::warn(TAG, sScroreboardRandom);
    std::srand(unsigned(std::time(0)));
    m_nGameStartInSec = nGameStartInSec;
    m_nGameEndInSec = nGameEndInSec;
    m_nGameCoffeeBreakStartInSec = nGameCoffeeBreakStartInSec;
    m_nGameCoffeeBreakEndInSec = nGameCoffeeBreakEndInSec;
    m_nFlagTimeLiveInSec = nFlagTimeLiveInSec;
    m_nAllDefenceFlags = 0;
    m_nAllTriesActivities = 0;
    m_nCostDefenceFlagInPoints10 = 10;
    m_nTeamCount = vTeamsConf.size();
    m_pEmployFlags = findWsjcppEmploy<EmployFlags>();

    m_mapTeamsStatuses.clear(); // possible memory leak
    for (unsigned int iteam = 0; iteam < vTeamsConf.size(); iteam++) {
        Ctf01dTeamDef teamConf = vTeamsConf[iteam];
        std::string sTeamId = teamConf.getId();

        m_mapTeamsStatuses[sTeamId] = new TeamStatusRow(sTeamId, nGameStartInSec, nGameEndInSec);
        m_mapTeamsStatuses[sTeamId]->setPlace(iteam+1);

        // random values of service for testing
        if (m_bRandom) {
            int nPoints = (std::rand() % 10000);
            m_mapTeamsStatuses[sTeamId]->setPoints(nPoints);
        }

        for (unsigned int iservice = 0; iservice < vServicesConf.size(); iservice++) {
            Ctf01dServiceDef service = vServicesConf[iservice];
            m_mapTeamsStatuses[sTeamId]->setServiceStatus(service.id(), ServiceStatusCell::SERVICE_DOWN);

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
    const std::vector<Ctf01dTeamDef> &vTeamsConf = pConfig->teamsConf();
    const std::vector<Ctf01dServiceDef> &vServices = pConfig->servicesConf();

    nlohmann::json jsonServicesStatistics;
    for (unsigned int iservice = 0; iservice < vServices.size(); iservice++) {
        Ctf01dServiceDef serviceConf = vServices[iservice];
        nlohmann::json serviceStatistics;
        m_mapServiceCostsAndStatistics[serviceConf.id()]->updateJsonServiceStatistics(serviceStatistics);
        jsonServicesStatistics[serviceConf.id()] = serviceStatistics;
    }
    m_jsonScoreboard["s_sta"] = jsonServicesStatistics;
    m_jsonScoreboard["sum_act"] = m_nAllTriesActivities;

    nlohmann::json jsonScoreboard;
    for (unsigned int iteam = 0; iteam < vTeamsConf.size(); iteam++) {
        Ctf01dTeamDef teamConf = vTeamsConf[iteam];
        std::string sTeamId = teamConf.getId();
        nlohmann::json teamData;
        teamData["place"] = m_mapTeamsStatuses[sTeamId]->getPlace();
        teamData["points"] = double(m_mapTeamsStatuses[sTeamId]->getPoints()) / 10.0;
        teamData["tries"] = 0;
        teamData["logo_last_updated"] = 0;
        nlohmann::json jsonServices;
        for (unsigned int iservice = 0; iservice < vServices.size(); iservice++) {
            Ctf01dServiceDef serviceConf = vServices[iservice];
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
        jsonScoreboard[teamConf.getId()] = teamData;
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
    std::string sResult = ServiceStatusCell::SERVICE_DOWN;
    int nState = std::rand() % 5;
    switch (nState) {
        case 0: sResult = ServiceStatusCell::SERVICE_UP; break;
        case 1: sResult = ServiceStatusCell::SERVICE_DOWN; break;
        case 2: sResult = ServiceStatusCell::SERVICE_MUMBLE; break;
        case 3: sResult = ServiceStatusCell::SERVICE_CORRUPT; break;
        case 4: sResult = ServiceStatusCell::SERVICE_SHIT; break;
    }
    return sResult;
}

void Ctf01dScoreboard::setServiceStatus(const std::string &sTeamId, const std::string &sServiceId, const std::string &sStatus) {
    std::lock_guard<std::mutex> lock(m_mutexJson);
    std::string sNewStatus = m_bRandom ? randomServiceStatus() : sStatus;

    std::map<std::string,TeamStatusRow *>::iterator it;
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
    std::map<std::string,TeamStatusRow *>::iterator it;
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
    const std::vector<Ctf01dServiceDef> &vServices = pConfig->servicesConf();
    m_nCostDefenceFlagInPoints10 = pConfig->getCostDefenceFlagInPoints10();

    WsjcppLog::info(TAG, "Loading alive flags...");
    std::vector<Ctf01dFlag> vFlagLives = m_pDatabase->listOfLiveFlags();
    for (unsigned int i = 0; i < vFlagLives.size(); i++) {
        Ctf01dFlag flag = vFlagLives[i];
        m_mapFlagsLive[flag.getValue()] = flag;
    }

    // load services statistics
    WsjcppLog::info(TAG, "Loading services statistics...");
    m_nAllDefenceFlags = 0;
    struct FlagsForService {
        std::string sServiceID;
        std::string sFirstBloodTeamID;
        long nFirstBloodTime;
        int nStolenFlags;
        int nDefenceFlags;
    };
    std::vector<FlagsForService> vFlags;
    for (unsigned int i = 0; i < vServices.size(); i++) {
        std::string sServiceID = vServices[i].id();
        FlagsForService f;
        f.sServiceID = sServiceID;
        f.nStolenFlags = m_pDatabase->numberOfStolenFlagsForService(sServiceID);
        f.nDefenceFlags = m_pDatabase->numberOfDefenceFlagForService(sServiceID);
        if (f.nStolenFlags > 0) {
            std::pair<std::string, long> fb = m_pDatabase->getFirstbloodFromStolenFlagsForService(sServiceID);
            f.sFirstBloodTeamID = fb.first;
            f.nFirstBloodTime = fb.second;
        }
        m_nAllDefenceFlags += f.nDefenceFlags;
        vFlags.push_back(f);
    }

    WsjcppLog::info(TAG, "Setting services statistics...");
    for (int i = 0; i < vFlags.size(); i++) {
        FlagsForService f = vFlags[i];
        m_mapServiceCostsAndStatistics[f.sServiceID]->setStolenFlagsForService(f.nStolenFlags);
        m_mapServiceCostsAndStatistics[f.sServiceID]->setDefenceFlagsForService(f.nDefenceFlags);
        if (f.nStolenFlags > 0) {
            m_mapServiceCostsAndStatistics[f.sServiceID]->setFirstBloodTeamId(f.sFirstBloodTeamID, f.nFirstBloodTime);
        }
    }

    WsjcppLog::info(TAG, "Setting teams statistics...");
    m_nAllTriesActivities = 0;
    std::map<std::string, TeamStatusRow *>::iterator it;
    for (it = m_mapTeamsStatuses.begin(); it != m_mapTeamsStatuses.end(); it++) {
        TeamStatusRow *pRow = it->second;

        int nTries = m_pDatabase->numberOfFlagAttempts(pRow->teamId());
        m_nAllTriesActivities += nTries;
        pRow->setTries(nTries);
        m_jsonScoreboard["scoreboard"][pRow->teamId()]["tries"] = nTries;

        for (unsigned int i = 0; i < vServices.size(); i++) {
            std::string sServiceID = vServices[i].id();

            // calculate defence
            int nDefenceFlags = m_pDatabase->numberOfFlagsDefense(pRow->teamId(), sServiceID);
            int nDefencePoints = m_pDatabase->sumPointsOfFlagsDefense(pRow->teamId(), sServiceID);
            pRow->setServiceDefenceFlagsAndPoints(sServiceID, nDefenceFlags, nDefencePoints);
            m_jsonScoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["def"] = nDefenceFlags;
            m_jsonScoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["pt_def"] = double(nDefencePoints) / 10.0;

            // calculate attack
            int nAttackFlags = m_pDatabase->numberOfFlagsStollen(pRow->teamId(), sServiceID);
            int nAttackPoints = m_pDatabase->sumPointsOfFlagsStollen(pRow->teamId(), sServiceID);
            pRow->setServiceAttackFlagsAndPoints(sServiceID, nAttackFlags, nAttackPoints);
            m_jsonScoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["att"] = nAttackFlags;
            m_jsonScoreboard["scoreboard"][pRow->teamId()]["ts_sta"][sServiceID]["pt_att"] = double(nAttackPoints) / 10.0;

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

int Ctf01dScoreboard::incrementAttackScore(const Ctf01dFlag &flag, const std::string &sTeamId) {
    std::lock_guard<std::mutex> lock(m_mutexJson);
    std::string sServiceId = flag.getServiceId();

    // TODO calculate
    // int nFlagPoints = m_mapServiceCostsAndStatistics[sServiceId]->getCostStolenFlag()*10; // one number after dot
    int nFlagPoints = 10;
    int nDateAction = WsjcppCore::getCurrentTimeInMilliseconds();
    // victim place in scroreboard
    std::map<std::string,TeamStatusRow *>::iterator it_victim;
    it_victim = m_mapTeamsStatuses.find(flag.getTeamId());
    int nVictimPlaceInScoreBoard = 0;
    if (it_victim != m_mapTeamsStatuses.end()) {
        nVictimPlaceInScoreBoard = it_victim->second->getPlace();
    }

    std::map<std::string,TeamStatusRow *>::iterator it;
    it = m_mapTeamsStatuses.find(sTeamId);
    if (it != m_mapTeamsStatuses.end()) {
        TeamStatusRow *pRow = it->second;
        int nThiefPlaceInScoreboard = pRow->getPlace();

        // motivation
        // WsjcppLog::info(TAG, "nVictimPlaceInScoreBoard " + std::to_string(nVictimPlaceInScoreBoard));
        // WsjcppLog::info(TAG, "nThiefPlaceInScoreboard " + std::to_string(nThiefPlaceInScoreboard));
        // WsjcppLog::info(TAG, "m_nTeamCount " + std::to_string(m_nTeamCount));
        float nMotivation = 1.0; // default
        if (nVictimPlaceInScoreBoard > nThiefPlaceInScoreboard) {
            nMotivation -= float(nVictimPlaceInScoreBoard - nThiefPlaceInScoreboard) / float(m_nTeamCount - 1);
        }
        nFlagPoints = nFlagPoints * nMotivation;
        // WsjcppLog::info(TAG, "nMotivation " + std::to_string(nMotivation));
        // WsjcppLog::info(TAG, "nFlagPoints " + std::to_string(nFlagPoints));

        m_pDatabase->insertToFlagsStolen(flag, sTeamId, nFlagPoints, nDateAction, nVictimPlaceInScoreBoard, nThiefPlaceInScoreboard);
        pRow->incrementAttack(sServiceId, nFlagPoints);
        pRow->updatePoints();
        m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["att"] = pRow->getAttackFlags(sServiceId);
        m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["pt_att"] = double(pRow->getAttackPoints(sServiceId)) / 10.0;
        m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["sla"] = pRow->calculateSLA(sServiceId);
        m_jsonScoreboard["scoreboard"][sTeamId]["points"] = double(pRow->getPoints()) / 10.0;
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
    return nFlagPoints;
}

void Ctf01dScoreboard::incrementDefenceScore(const Ctf01dFlag &flag) {
    std::lock_guard<std::mutex> lock(m_mutexJson);

    std::string sTeamId = flag.getTeamId();
    std::string sServiceId = flag.getServiceId();
    int nFlagPoints = m_nCostDefenceFlagInPoints10;
    m_pDatabase->insertToFlagsDefence(flag, nFlagPoints);

    std::map<std::string,TeamStatusRow *>::iterator it;
    it = m_mapTeamsStatuses.find(sTeamId);
    if (it != m_mapTeamsStatuses.end()) {
        TeamStatusRow *pRow = it->second;
        pRow->incrementDefence(sServiceId, nFlagPoints);
        m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["def"] = pRow->getDefenceFlags(sServiceId);
        m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["pt_def"] = double(pRow->getDefencePoints(sServiceId)) / 10.0;
        m_jsonScoreboard["scoreboard"][sTeamId]["points"] = double(pRow->getPoints()) / 10.0;
        sortPlaces();
    }

    std::map<std::string, Ctf01dServiceStatistics *>::iterator it2;
    it2 = m_mapServiceCostsAndStatistics.find(sServiceId);
    if (it2 != m_mapServiceCostsAndStatistics.end()) {
        m_nAllDefenceFlags++;
        it2->second->doIncrementDefenceFlagsForService();
        updateServicesStatistics();
    }
}

void Ctf01dScoreboard::incrementFlagsPuttedAndServiceUp(const Ctf01dFlag &flag) {
    std::string sServiceId = flag.getServiceId();
    std::string sTeamId = flag.getTeamId();
    std::string sNewStatus = m_bRandom ? randomServiceStatus() : ServiceStatusCell::SERVICE_UP;

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
    std::map<std::string,TeamStatusRow *>::iterator it;
    it = m_mapTeamsStatuses.find(sTeamId);
    if (it != m_mapTeamsStatuses.end()) {
        TeamStatusRow *pRow = it->second;
        if (pRow->serviceStatus(sServiceId) != sNewStatus) {
            pRow->setServiceStatus(sServiceId, sNewStatus);
        }
        pRow->updatePoints();
	    m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["status"] = sNewStatus;
        m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["sla"] = pRow->calculateSLA(sServiceId);
        m_jsonScoreboard["scoreboard"][sTeamId]["points"] = double(pRow->getPoints()) / 10.0;
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

    std::map<std::string,TeamStatusRow *>::iterator it;
    it = m_mapTeamsStatuses.find(flag.getTeamId());
    if (it != m_mapTeamsStatuses.end()) {
        TeamStatusRow *pRow = it->second;
        if (pRow->serviceStatus(sServiceId) != sNewStatus) {
            pRow->setServiceStatus(sServiceId, sNewStatus);
        }
        pRow->incrementPutFlagFail(sServiceId);
        pRow->updatePoints();
        m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["status"] = sNewStatus;
        m_jsonScoreboard["scoreboard"][sTeamId]["ts_sta"][sServiceId]["sla"] = pRow->calculateSLA(sServiceId);
        m_jsonScoreboard["scoreboard"][sTeamId]["points"] = float(pRow->getPoints()) / 10.0;
        sortPlaces();
    }
}

void Ctf01dScoreboard::updateScore(const std::string &sTeamId, const std::string &sServiceId) {
    std::lock_guard<std::mutex> lock(m_mutexJson);
    std::map<std::string,TeamStatusRow *>::iterator it;
    it = m_mapTeamsStatuses.find(sTeamId);
    if (it != m_mapTeamsStatuses.end()) {
        TeamStatusRow *pRow = it->second;
        // pRow->updateScore(sServiceId);
        m_jsonScoreboard["scoreboard"][sTeamId]["points"] = float(pRow->getPoints()) / 10.0;
        sortPlaces();
    }
}

std::string Ctf01dScoreboard::serviceStatus(const std::string &sTeamId, const std::string &sServiceId) {
    std::map<std::string,TeamStatusRow *>::iterator it;
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
        std::map<std::string,TeamStatusRow *>::iterator it1;
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
        std::map<std::string,TeamStatusRow *>::iterator it1;
        for (it1 = m_mapTeamsStatuses.begin(); it1 != m_mapTeamsStatuses.end(); it1++) {
            TeamStatusRow *pTeamStatus = it1->second;
            std::string sTeamId_ = pTeamStatus->teamId();

            // std::cout << sTeamNum << ": result: score: " << pTeamStatus->score() << ", place: " << pTeamStatus->getPlace() << "\n";
            m_jsonScoreboard["scoreboard"][sTeamId_]["points"] = double(pTeamStatus->getPoints()) / 10.0;
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
    long nCurrentTime = WsjcppCore::getCurrentTimeInMilliseconds() - m_nFlagTimeLiveInSec*1000;
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

std::string Ctf01dScoreboard::toString(){
    // TODO mutex
    std::string sResult = "";
    std::map<std::string, TeamStatusRow *>::iterator it;
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
