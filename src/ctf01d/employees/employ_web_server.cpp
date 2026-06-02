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

#include "employ_web_server.h"
#include "ctf01d/employees/employ_config.h"
#include "ctf01d/employees/employ_team_logos.h"
#include "ctf01d/objects/ctf01d_team_logo.h"
#include <wsjcpp_core.h>
#include <fstream>
#include <cstring>
#include <optional>
#include <regex>

#include "WebSocketServer.h"  // libhv
#include "EventLoop.h"  // libhv
#include "htime.h"  // libhv
#include "hssl.h"  // libhv
#include "hlog.h"  // libhv

REGISTRY_WSJCPP_EMPLOY(EmployWebServer)

EmployWebServer::EmployWebServer()
: WsjcppEmployBase({ EmployWebServer::name() }, { EmployConfig::name() }) {
  m_sApiPathPrefix = "/api/v1/";
  m_sTeamLogoPrefix = "/team-logo/";
  m_nTeamLogoPrefixLength = m_sTeamLogoPrefix.size();
}

bool EmployWebServer::init(const std::string &name, bool bSilent) {
  WsjcppLog::info(TAG, "init");
  return true;
}

bool EmployWebServer::deinit(const std::string &name, bool bSilent) {
  WsjcppLog::info(TAG, "deinit");
  return true;
}

using namespace hv;

void EmployWebServer_custom_logger(int level, const char *msg, int len) {
  std::string TAG = "http";
  std::string message(msg, len - 1); // remove last '\n' character
  switch (level) {
  case LOG_LEVEL_DEBUG:
    WsjcppLog::info(TAG, "debug: " + message);
    break;
  case LOG_LEVEL_INFO:
    WsjcppLog::info(TAG, message);
    break;
  case LOG_LEVEL_WARN:
    WsjcppLog::warn(TAG, message);
    break;
  case LOG_LEVEL_ERROR:
    WsjcppLog::err(TAG, message);
    break;
  case LOG_LEVEL_FATAL:
    WsjcppLog::throw_err(TAG, message);
    break;
  default:
    WsjcppLog::info(TAG, "Unknow level: " + message);
  }
}

int EmployWebServer::start() {

  EmployConfig *pEmployConfig = findWsjcppEmploy<EmployConfig>();

  m_sScoreboardHtmlFolder = pEmployConfig->scoreboardHtmlFolder();
  updateJsonCache();

  WsjcppLog::ok(TAG, "Starting scoreboard on http://localhost:" + std::to_string(pEmployConfig->scoreboardPort()) + "/");

  {
    logger_t *pLogger = hv_default_logger();
    logger_set_handler(pLogger, EmployWebServer_custom_logger);
    logger_set_format(pLogger, "%s"); // removing time and log level

    // Test the log
    hlogi("This is an info message.");
  }

  m_pHttpService = std::make_shared<hv::HttpService>();

  // static files
  m_pHttpService->document_root = "./html";
  m_pHttpService->GET("*", std::bind(&EmployWebServer::httpWebFolder, this, std::placeholders::_1, std::placeholders::_2));

  hv::HttpServer server(m_pHttpService.get());
  server.setPort(pEmployConfig->scoreboardPort());
  server.setThreadNum(4);
  server.run();

  // TODO: stop all threads

  /*while(1) {
    Log::info(TAG, "wait 2 minutes");
    std::this_thread::sleep_for(std::chrono::minutes(2));
    Log::info(TAG, "wait ended");
  }*/

  return 0;
}

void EmployWebServer::updateJsonCache() {
  auto config = findWsjcppEmploy<EmployConfig>();

  nlohmann::json jsonGame;
  nlohmann::json jsonTeams;

  jsonGame["game_name"] = config->gameName();
  jsonGame["game_start"] = WsjcppCore::formatTimeUTC(config->gameStartUTCInSec()) + " (UTC)";
  jsonGame["game_end"] = WsjcppCore::formatTimeUTC(config->gameEndUTCInSec()) + " (UTC)";
  jsonGame["game_has_coffee_break"] = config->gameHasCoffeeBreak();
  jsonGame["game_coffee_break_start"] = WsjcppCore::formatTimeUTC(config->gameCoffeeBreakStartUTCInSec()) + " (UTC)";
  jsonGame["game_coffee_break_end"] = WsjcppCore::formatTimeUTC(config->gameCoffeeBreakEndUTCInSec()) + " (UTC)";
  jsonGame["teams"] = nlohmann::json::array();
  jsonGame["services"] = nlohmann::json::array();

  for (unsigned int i = 0; i < config->servicesConf().size(); i++) {
    ctf01d::service_config serviceConf = config->servicesConf()[i];
    if (serviceConf.isEnabled()) {
      nlohmann::json serviceInfo;
      serviceInfo["id"] = serviceConf.id();
      serviceInfo["name"] = serviceConf.name();
      serviceInfo["round_time_in_sec"] = serviceConf.round_in_seconds();
      jsonGame["services"].push_back(serviceInfo);
    }
  }

  for (unsigned int i = 0; i < config->teamsConf().size(); i++) {
      ctf01d::team_config teamConf = config->teamsConf()[i];
      nlohmann::json teamInfo;
      teamInfo["id"] = teamConf.id();
      teamInfo["name"] = teamConf.name();
      teamInfo["ip_address"] = teamConf.ip_or_host();
      teamInfo["logo"] = "./team-logo/" + teamConf.id();
      teamInfo["logo"] = "./team-big-logo/" + teamConf.id();
      teamInfo["logo_last_write_time"] = teamConf.getLogoLastWriteTime();

      jsonGame["teams"].push_back(teamInfo);
      jsonTeams["teams"].push_back(teamInfo);
  }

  m_sCacheResponseGameJson = jsonGame.dump();
  m_sCacheResponseTeamsJson = jsonTeams.dump();
}

int EmployWebServer::httpWebFolder(HttpRequest* req, HttpResponse* resp) {
  std::string sOriginalRequestPath = req->path;
  std::string request_path;

  // remove get params from path
  std::size_t nFoundGetParams = sOriginalRequestPath.rfind("?");
  if (nFoundGetParams != std::string::npos) {
    request_path = sOriginalRequestPath.substr(0, nFoundGetParams);
  } else {
    request_path = sOriginalRequestPath;
  }
  request_path = wsjcpp::normalizeFilePath(request_path);

  // WsjcppLog::info(TAG, "request_path = " + request_path);
  if (request_path == "/flag") { // Public endpoint. Allowed without authorization.
    return this->httpApiV1Flag(req, resp);
  }

  if (request_path.rfind(m_sTeamLogoPrefix, 0) == 0) {
    return httpTeamLogos(request_path, req, resp);
  }

  if (request_path.rfind(m_sApiPathPrefix, 0) == 0) {
    if (request_path == "/api/v1/game") { // Public endpoint. Allowed without authorization.
      return this->httpApiV1Game(req, resp);
    } else if (request_path == "/api/v1/game/current-time") { // Public endpoint. Allowed without authorization.
      return this->httpApiGameCurrentTime(req, resp);
    } else if (request_path == "/api/v1/scoreboard") { // Public endpoint. Allowed without authorization.
      return this->httpApiV1Scoreboard(req, resp);
    } else if (request_path == "/api/v1/myip") { // it's ok. Because network game is public space. This endpoint need for automatic configuration network.
      return this->httpApiV1MyIp(req, resp);
    } else if (request_path == "/api/v1/teams") { // Public endpoint. Allowed without authorization.
      return this->httpApiV1Teams(req, resp);
    }
    return this->httpApiV1GetPaths(req, resp);
  }

  if (request_path == "/") {
      request_path = "/index.html";
  }

  std::string filepath = wsjcpp::normalizeFilePath(m_sScoreboardHtmlFolder + "/" + request_path);
  if (WsjcppCore::dirExists(filepath)) {
      return 404;
  }
  if (WsjcppCore::fileExists(filepath)) {
      return resp->File(filepath.c_str());
  }

  std::string sResPath = wsjcpp::normalizeFilePath("./data_sample/html/" + request_path);
  if (WsjcppResourcesManager::has(sResPath)) {
      WsjcppResourceFile *pFile = WsjcppResourcesManager::get(sResPath);
      resp->Data(
          (void *)pFile->getBuffer(),
          pFile->getBufferSize(),
          true // nocopy
      );
      resp->SetContentTypeByFilename(sResPath.c_str());
      return 200;
  }
  return 404; // Not found
}

int EmployWebServer::httpApiV1Game(HttpRequest* req, HttpResponse* resp) {
  // std::cout << m_sCacheResponseGameJson << std::endl;
  resp->Data(
    (void *)(m_sCacheResponseGameJson.c_str()),
    m_sCacheResponseGameJson.length(),
    true // nocopy
  );
  resp->SetContentTypeByFilename("game.json");
  return 200;
}

int EmployWebServer::httpApiGameCurrentTime(HttpRequest* req, HttpResponse* resp) {
  // TODO maybe need optimization keep json response for every second.
  auto config = findWsjcppEmploy<EmployConfig>();
  auto now = std::chrono::system_clock::now().time_since_epoch();
  int nCurrentTimeSec = std::chrono::duration_cast<std::chrono::seconds>(now).count();
  nlohmann::json current_time;
  current_time["current-time"] = nCurrentTimeSec - config->gameStartUTCInSec();
  std::string json_str = current_time.dump();
  resp->Data(
      (void *)(json_str.c_str()),
      json_str.length(),
      false // force copy
  );
  resp->SetContentTypeByFilename("current-time.json");
  return 200;
}

int EmployWebServer::httpApiV1Teams(HttpRequest* req, HttpResponse* resp) {
  resp->Data(
    (void *)(m_sCacheResponseTeamsJson.c_str()),
    m_sCacheResponseTeamsJson.length(),
    true // nocopy
  );
  resp->SetContentTypeByFilename("teams.json");
  return 200;
}

int EmployWebServer::httpApiV1MyIp(HttpRequest* req, HttpResponse* resp) {
  resp->json["myip"] = req->client_addr.ip;
  return 200;
}

int EmployWebServer::httpApiV1Scoreboard(HttpRequest* req, HttpResponse* resp) {
  auto teamLogos = findWsjcppEmploy<EmployTeamLogos>();
  auto config = findWsjcppEmploy<EmployConfig>();
  teamLogos->updateLastChangeTime();
  nlohmann::json jsonScoreboard = config->scoreboard()->toJson();
  teamLogos->updateScoreboardJson(jsonScoreboard);
  std::string sScoreboardJson = jsonScoreboard.dump();
  resp->Data(
      (void *)(sScoreboardJson.c_str()),
      sScoreboardJson.length(),
      false // nocopy - force copy
  );
  resp->SetContentTypeByFilename("scoreboard.json");
  return 200;
}

int EmployWebServer::httpApiV1GetPaths(HttpRequest* req, HttpResponse* resp) {
  // TODO
  return resp->Json(m_pHttpService->Paths());
}

int EmployWebServer::httpApiV1Flag(HttpRequest* req, HttpResponse* resp) {
  auto config = findWsjcppEmploy<EmployConfig>();
  auto now = std::chrono::system_clock::now().time_since_epoch();
  int nCurrentTimeSec = std::chrono::duration_cast<std::chrono::seconds>(now).count();
  std::string sRequestIP = req->client_addr.ip;
  std::string sRequestIP_MsgSuffix = " (" + sRequestIP + ")";

  if (nCurrentTimeSec < config->gameStartUTCInSec()) {
    const std::string sErrorMsg = " Error(-8): Game not started yet";
    WsjcppLog::err(TAG, sErrorMsg + sRequestIP_MsgSuffix);
    resp->String(sErrorMsg);
    return 400;
  }

  if (config->gameHasCoffeeBreak()
    && nCurrentTimeSec > config->gameCoffeeBreakStartUTCInSec()
    && nCurrentTimeSec < config->gameCoffeeBreakEndUTCInSec()
  ) {
    static const std::string sErrorMsg = "Error(-8): Game on coffee break now";
    WsjcppLog::err(TAG, sErrorMsg + sRequestIP_MsgSuffix);
    resp->String(sErrorMsg);
    return 400;
  }

  if (nCurrentTimeSec > config->gameEndUTCInSec()) {
    static const std::string sErrorMsg = "Error(-9): Game already ended";
    WsjcppLog::err(TAG, sErrorMsg + sRequestIP_MsgSuffix);
    resp->String(sErrorMsg);
    return 400;
  }

  std::string sTeamId = req->GetParam("teamid");
  sTeamId = WsjcppCore::trim(sTeamId);
  sTeamId = WsjcppCore::toLower(sTeamId);
  std::string sFlag = req->GetParam("flag");
  sFlag = WsjcppCore::trim(sFlag);
  sFlag = WsjcppCore::toLower(sFlag);

  if (sTeamId == "") {
    static const std::string sErrorMsg = "Error(-10): Not found get-parameter 'teamid' or parameter is empty";
    WsjcppLog::err(TAG, sErrorMsg + sRequestIP_MsgSuffix);
    resp->String(sErrorMsg);
    return 400;
  }

  if (sFlag == "") {
    static const std::string sErrorMsg = "Error(-11): Not found get-parameter 'flag' or parameter is empty";
    WsjcppLog::err(TAG, sErrorMsg + sRequestIP_MsgSuffix);
    resp->String(sErrorMsg);
    return 400;
  }

  // TODO optimize
  bool bTeamFound = false;
  for (unsigned int i_team = 0; i_team < config->teamsConf().size(); i_team++) {
    ctf01d::team_config teamConf = config->teamsConf()[i_team];
    if (teamConf.id() == sTeamId) {
      bTeamFound = true;
    }
  }

  if (!bTeamFound) {
      static const std::string sErrorMsg = "Error(-130): this is team not found";
      WsjcppLog::err(TAG, sErrorMsg + sRequestIP_MsgSuffix);
      resp->String(sErrorMsg);
      return 400;
  }

  // TODO test speed of regexp and if will be simple compare every symbol.
  const static std::regex reFlagFormat("c01d[a-f0-9]{4,4}-[a-f0-9]{4,4}-[a-f0-9]{4,4}-[a-f0-9]{4,4}-[a-f0-9]{4,4}[0-9]{8,8}");
  if (!std::regex_match(sFlag, reFlagFormat)) {
      static const std::string sErrorMsg = "Error(-140): flag has wrong format";
      WsjcppLog::err(TAG, sErrorMsg + sRequestIP_MsgSuffix);
      resp->String(sErrorMsg);
      return 400;
  }
  config->scoreboard()->incrementTries(sTeamId);

  findWsjcppEmploy<EmployDatabase>()->insertFlagAttempt(sTeamId, sFlag, sRequestIP);

  // TODO m_pEmployFlags->insertFlagAttempt(sTeamId, sFlag);

  Ctf01dFlag flag;
  if (!config->scoreboard()->findFlagLive(sFlag, flag)) {
      static const std::string sErrorMsg = "Error(-150): flag is too old or flag never existed or flag already stole.";
      WsjcppLog::err(TAG, sErrorMsg + ". Received flag {" + sFlag + "} from {" + sTeamId + "}" + sRequestIP_MsgSuffix);
      resp->String(sErrorMsg);
      return 403;
  }

  long nCurrentTimeMSec = (long)nCurrentTimeSec;
  nCurrentTimeMSec = nCurrentTimeMSec*1000;

  if (flag.getTimeEndInMs() < nCurrentTimeMSec) {
      // TODO
      static const std::string sErrorMsg = "Error(-151): flag is too old";
      WsjcppLog::err(TAG, sErrorMsg + ". Received flag {" + sFlag + "} from {" + sTeamId + "}" + sRequestIP_MsgSuffix);
      resp->String(sErrorMsg);
      return 403;
  }

  // if (flag.teamStole() == sTeamId) {
  //     response.forbidden().sendText("Error(-160): flag already stole by your team");
  //     WsjcppLog::err(TAG, "Error(-160): Received flag {" + sFlag + "} from {" + sTeamId + "} (flag already stole by your team)");
  //     return true;
  // }

  if (flag.getTeamId() == sTeamId) {
      static const std::string sErrorMsg = "Error(-180): this is your flag";
      WsjcppLog::err(TAG, sErrorMsg + ". Received flag {" + sFlag + "} from {" + sTeamId + "}" + sRequestIP_MsgSuffix);
      resp->String(sErrorMsg);
      return 403;
  }

  std::string sServiceStatus = config->scoreboard()->serviceStatus(sTeamId, flag.getServiceId());

  // std::cout << "sServiceStatus: " << sServiceStatus << "\n";

  if (sServiceStatus != Ctf01dServiceStatusCell::SERVICE_UP) {
      static const std::string sErrorMsg = "Error(-190): Your same service is dead. Try later.";
      WsjcppLog::err(TAG, sErrorMsg + ". Received flag {" + sFlag + "} from {" + sTeamId + "}" + sRequestIP_MsgSuffix);
      resp->String(sErrorMsg);
      return 403;
  }

  // TODO light update scoreboard
  // incrementAttackScore performs the dedup check under its own mutex,
  // so check-then-insert is atomic against concurrent submissions.
  std::optional<int> oPoints = config->scoreboard()->incrementAttackScore(flag, sTeamId);
  if (!oPoints.has_value()) {
      static const std::string sErrorMsg = "Error(-170): flag already stolen by your team";
      WsjcppLog::err(TAG, sErrorMsg + ". Received flag {" + sFlag + "} from {" + sTeamId + "}" + sRequestIP_MsgSuffix);
      resp->String(sErrorMsg);
      return 403;
  }
  std::string sPoints = std::to_string(oPoints.value());

  std::string sResponse = "Accepted: Received flag {" + sFlag + "} from {" + sTeamId + "} (Accepted + " + sPoints + ")";
  WsjcppLog::ok(TAG, sResponse + sRequestIP_MsgSuffix);
  resp->Data(
      (void *)(sResponse.c_str()),
      sResponse.size(),
      false // copy buffer
  );
  resp->content_type = TEXT_PLAIN;
  return 200;
}

int EmployWebServer::httpTeamLogos(const std::string &request_path, HttpRequest* req, HttpResponse* resp) {
  std::string sTeamId = request_path.substr(m_nTeamLogoPrefixLength, request_path.length() - m_nTeamLogoPrefixLength);
  auto teamLogos = findWsjcppEmploy<EmployTeamLogos>();
  Ctf01dTeamLogo *pLogo = teamLogos->findTeamLogo(sTeamId);
  if (pLogo == nullptr) {
    return 404;
  }
  resp->Data(
    pLogo->pBuffer,
    pLogo->nBufferSize,
    true // nocopy
  );
  resp->SetContentTypeByFilename(pLogo->sFilename.c_str());
  return 200;
}
