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

#pragma once

#include <wsjcpp_employees.h>
#include <string>
#include "HttpService.h" // libhv

class EmployWebServer : public WsjcppEmployBase {
public:
  EmployWebServer();
  static std::string name() { return "EmployWebServer"; }
  virtual bool init(const std::string &name, bool bSilent) override;
  virtual bool deinit(const std::string &name, bool bSilent) override;

  int start();

private:
  std::string TAG;

  void updateJsonCache();

  int httpWebFolder(HttpRequest* req, HttpResponse* resp);
  int httpApiV1Game(HttpRequest* req, HttpResponse* resp);
  int httpApiGameCurrentTime(HttpRequest* req, HttpResponse* resp);
  int httpApiV1Teams(HttpRequest* req, HttpResponse* resp);
  int httpApiV1MyIp(HttpRequest* req, HttpResponse* resp);
  int httpApiV1Scoreboard(HttpRequest* req, HttpResponse* resp);
  int httpApiV1GetPaths(HttpRequest* req, HttpResponse* resp);
  int httpApiV1Flag(HttpRequest* req, HttpResponse* resp);
  int httpApiV1Metrics(HttpRequest* req, HttpResponse* resp);
  int httpTeamLogos(const std::string &request_path, HttpRequest* req, HttpResponse* resp);

  std::shared_ptr<hv::HttpService> m_pHttpService;
  std::string m_sApiPathPrefix;
  std::string m_sTeamLogoPrefix;
  int m_nTeamLogoPrefixLength;
  std::string m_sIndexHtml;
  std::string m_sScoreboardHtmlFolder;
  std::string m_sCacheResponseGameJson;
  std::string m_sCacheResponseTeamsJson;
};
