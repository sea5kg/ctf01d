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

#ifndef EMPLOY_SCOREBOARD_H
#define EMPLOY_SCOREBOARD_H

#include <wsjcpp_employees.h>
#include <json.hpp>

class Ctf01dServiceStatistics {
    public:
        Ctf01dServiceStatistics(const std::string &sServiceId);
        int getAllStolenFlagsForService();
        void doIncrementStolenFlagsForService(int nAllStolenFlags);
        void setStolenFlagsForService(int nStolenFlags);

        int getAllDefenseFlagsForService();
        void doIncrementDefenseFlagsForService();

        void setDefenseFlagsForService(int nAllDefenseFlagsForService);

        std::string getFirstBloodTeamId();
        long getFirstBloodTime();
        void setFirstBloodTeamId(const std::string &sFirstBlood, long nDateACtion);
        void updateJsonServiceStatistics(nlohmann::json &jsonCosts);

    private:
        std::string TAG;
        std::string m_sServiceId;
        std::string m_sFirstBloodTeamId;
        long m_nFirstBloodTimeInSeconds;

        int m_nAllStolenFlagsForService;
        int m_nAllDefenseFlagsForService;
};

// ---------------------------------------------------------------------

class ServiceStatusCell {
    public:
        // enum for service status
        static std::string SERVICE_UP;
        static std::string SERVICE_DOWN;
        static std::string SERVICE_MUMBLE;
        static std::string SERVICE_CORRUPT;
        static std::string SERVICE_SHIT;
        static std::string SERVICE_WAIT;
        static std::string SERVICE_COFFEEBREAK;

        ServiceStatusCell(const std::string &sServiceId);
        const std::string &serviceId();

        void setDefenseFlags(int nDefenseFlags);
        int getDefenseFlags();
        void incrementDefenseFlags();

        void setDefensePoints(int nDefensePoints);
        int getDefensePoints();
        void addDefensePoints(int nDefensePoints);

        void setAttackFlags(int nAttackFlags);
        int getAttackFlags();
        void incrementAttackFlags();

        void setAttackPoints(int nAttackPoints);
        int getAttackPoints();
        void addAttackPoints(int nAttackPoints);

        void setFlagsPutAllResultsCounter(int nFlagsPutAllResultsCounter);
        void setFlagsPutSuccessResultsCounter(int nFlagsPutSuccessResultsCounter);
        void incrementPutFlagSuccess();
        void incrementPutFlagFail();
        int calculateSLA();

        void setStatus(const std::string &sStatus);
        std::string status();

    private:
        std::string TAG;
        std::mutex m_mutexServiceStatus;
        std::string m_sServiceId;
        std::string m_sStatus; // may be char[10] ?
        int m_nDefenseFlags;
        int m_nAttackFlags;
        int m_nAttackPoints;
        int m_nDefensePoints;
        int m_nUpPointTimeInSec;

        // for SLA / uptime
        int m_nFlagsPutAllResultsCounter;
        int m_nFlagsPutSuccessResultsCounter;
};

class TeamStatusRow {
    public:
        TeamStatusRow(const std::string &sTeamId, int nGameStartInSec, int nGameEndInSec);
        const std::string &teamId();

        void setPlace(int nPlace);
        int getPlace();

        void setPoints(int nPoints);
        int getPoints();

        void setServiceStatus(const std::string &sServiceId, std::string sStatus);
        std::string serviceStatus(const std::string &sServiceId);

        void setTries(int nScore);
        int tries();

        std::string servicesToString();

        void incrementDefense(const std::string &sServiceId, int nFlagPoints);
        int getDefenseFlags(const std::string &sServiceId);
        int getDefensePoints(const std::string &sServiceId);
        void setServiceDefenseFlagsAndPoints(const std::string &sServiceId, int nDefenseFlags, int nDefensePoints);

        void incrementAttack(const std::string &sServiceId, int nFlagPoints);
        void setServiceAttackFlagsAndPoints(const std::string &sServiceId, int nAttackFlags, int nAttackPoints);
        int getAttackFlags(const std::string &sServiceId);
        int getAttackPoints(const std::string &sServiceId);

        void setServiceFlagsForCalculateSLA(const std::string &sServiceId, int nPutsFlagsAllResults, int nPutsFlagsSuccessResults);
        void incrementPutFlagSuccess(const std::string &sServiceId);
        void incrementPutFlagFail(const std::string &sServiceId);
        int calculateSLA(const std::string &sServiceId);

        void updatePoints();

    private:
        std::mutex m_mutex;
        std::string TAG;
        std::string m_sTeamId;
        int m_nPlace;
        int m_nPoints;
        int m_nTries;
        std::map<std::string, ServiceStatusCell *> m_mapServicesStatus;
};

class EmployScoreboard : public WsjcppEmployBase {
    public:
        EmployScoreboard();
        static std::string name() { return "EmployScoreboard"; }
        virtual bool init(const std::string &sName, bool bSilent) override;
        virtual bool deinit(const std::string &sName, bool bSilent) override;


    private:
        std::string TAG;

        std::map<std::string, Ctf01dServiceStatistics *> m_mapServiceCostsAndStatistics;
};

#endif // EMPLOY_SCOREBOARD_H
