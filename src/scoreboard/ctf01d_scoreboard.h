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

#ifndef CTF01D_SCOREBOARD_H
#define CTF01D_SCOREBOARD_H

#include <employ_flags.h>
#include <employ_scoreboard.h>
#include <employ_database.h>
#include <string>
#include <json.hpp>


class Ctf01dScoreboard {
    public:

        Ctf01dScoreboard(
            bool bRandom,
            int nGameStartInSec,
            int nGameEndInSec,
            int nGameCoffeeBreakStartInSec,
            int nGameCoffeeBreakEndInSec,
            int nFlagTimeLiveInSec
        );

        void setServiceStatus(const std::string &sTeamId, const std::string &sServiceId, const std::string &sStatus);
        void incrementTries(const std::string &sTeamId);
        void initStateFromStorage();

        int incrementAttackScore(const Ctf01dFlag &flag, const std::string &sTeamId);
        void incrementDefenceScore(const Ctf01dFlag &flag);
        void incrementFlagsPuttedAndServiceUp(const Ctf01dFlag &flag);
        void insertFlagPutFail(const Ctf01dFlag &flag, const std::string &sServiceStatus, const std::string &sDescrStatus);
        void updateScore(const std::string &sTeamId, const std::string &sServiceId);
        std::string serviceStatus(const std::string &sTeamId, const std::string &sServiceId);

        std::vector<Ctf01dFlag> outdatedFlagsLive(const std::string &sTeamId, const std::string &sServiceId);
        bool findFlagLive(const std::string &sFlagValue, Ctf01dFlag &flag);
        void removeFlagLive(const Ctf01dFlag &flag);

        std::string toString();
        const nlohmann::json &toJson();

    private:
        std::string TAG;
        EmployFlags *m_pEmployFlags;
        EmployDatabase *m_pDatabase;
        int m_nCostDefenceFlagInPoints10;
        int m_nGameStartInSec;
        int m_nGameEndInSec;
        int m_nGameCoffeeBreakStartInSec;
        int m_nGameCoffeeBreakEndInSec;
        int m_nFlagTimeLiveInSec;
        int m_nTeamCount;

        void sortPlaces(); // TODO merge this function with update costs
        void updateServicesStatistics();

        std::map<std::string, Ctf01dServiceStatistics *> m_mapServiceCostsAndStatistics;
        int m_nAllDefenceFlags;

        std::string randomServiceStatus();
        bool m_bRandom;
        std::map<std::string, TeamStatusRow *> m_mapTeamsStatuses;

        std::mutex m_mutexJson;
        nlohmann::json m_jsonScoreboard; // prepare data for scoreboard
        void initJsonScoreboard();
        void updateJsonScoreboard();
        // nlohmann::json m_jsonGF; // prepare data for flags costs

        // flags live for fast check
        std::mutex m_mutexFlagsLive;
        std::map<std::string, Ctf01dFlag> m_mapFlagsLive; // Must be in samewhere in storage
};

#endif // CTF01D_SCOREBOARD_H
