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

#ifndef EMPLOY_TEAM_LOGOS_H
#define EMPLOY_TEAM_LOGOS_H

#include <wsjcpp_employees.h>
#include <json.hpp>

struct Ctf01dTeamLogo {
    std::string sTeamId;
    char *pBuffer;
    int nBufferSize;
    std::string sFilename;
    std::string sFilepath;
    long nLastWriteTime;
};

class EmployTeamLogos : public WsjcppEmployBase {
    public:
        EmployTeamLogos();
        static std::string name() { return "EmployTeamLogos"; }
        virtual bool init() override;
        virtual bool deinit() override;
        bool loadTeamLogo(const std::string &sTeamId, const std::string &sFilepath);
        Ctf01dTeamLogo *findTeamLogo(const std::string &sTeamId);
        bool updateLastWriteTime();
        void updateScorebordJson(nlohmann::json &jsonScoreboard);

    private:
        std::string TAG;
        std::map<std::string, Ctf01dTeamLogo *> m_mapTeamLogos;
        int m_nLastUpdateWriteTimeLogosInSec;
};

#endif // EMPLOY_TEAM_LOGOS_H
