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

#ifndef CTF01D_HTTP_SERVER_H
#define CTF01D_HTTP_SERVER_H

#include <string>
#include <json.hpp>
#include "HttpService.h"
#include <employ_config.h>
#include <employ_team_logos.h>
#include <employ_flags.h>

class Ctf01dHttpServer {
    public:
        Ctf01dHttpServer();
        hv::HttpService *getService();
        int httpApiV1GetPaths(HttpRequest* req, HttpResponse* resp);
        int httpAdmin(HttpRequest* req, HttpResponse* resp);
        int httpWebFolder(HttpRequest* req, HttpResponse* resp);
        int httpApiV1Flag(HttpRequest* req, HttpResponse* resp);
        int httpApiV1Scoreboard(HttpRequest* req, HttpResponse* resp);
        int httpApiV1Game(HttpRequest* req, HttpResponse* resp);
        int httpApiV1MyIp(HttpRequest* req, HttpResponse* resp);

    private:
        std::string TAG;
        std::string m_sApiPathPrefix;
        std::string m_sTeamLogoPrefix;
        int m_nTeamLogoPrefixLength;
        hv::HttpService *m_pHttpService;

        EmployConfig *m_pConfig;
        EmployFlags *m_pEmployFlags;
        EmployDatabase *m_pEmployDatabase;
        EmployTeamLogos *m_pTeamLogos;

        std::string m_sIndexHtml;
        std::string m_sScoreboardHtmlFolder;

        nlohmann::json m_jsonGame;
        std::string m_sCacheResponseGameJson;
        nlohmann::json m_jsonTeams; // prepare data for list of teams
        std::string m_sCacheResponseTeamsJson;
};

#endif // CTF01D_HTTP_SERVER_H