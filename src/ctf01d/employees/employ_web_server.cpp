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

#include <fstream>
#include <atomic>
#include <cstring>
#include <initializer_list>
#include <optional>
#include <regex>
#include <sstream>
#include <utility>
#include <vector>
#include <wsjcpp_employees.h>
#include <wsjcpp_core.h>
#include "ctf01d/include/ctf01d_web_server.h"
#include "ctf01d/include/ctf01d_activities.h"
#include "ctf01d/include/ctf01d_config.h"
#include "ctf01d/include/ctf01d_images.h"
#include "ctf01d/utils/ctf01d_logger.h"
#include "ctf01d/objects/ctf01d_service_status_cell.h"

// libhv includes
#include "HttpService.h" // libhv
#include "WebSocketServer.h"  // libhv
#include "EventLoop.h"  // libhv
#include "htime.h"  // libhv
#include "hssl.h"  // libhv
#include "hlog.h"  // libhv
#include "hbase.h"  // libhv: hv_wildcard_match

class employ_web_server : public WsjcppEmployBase, public ctf01d::web_server {
public:
  employ_web_server();
  virtual bool init(const std::string &name, bool bSilent) override;
  virtual bool deinit(const std::string &name, bool bSilent) override;

  // IWebServer
  virtual int start() override;
  virtual void set_metrics_enabled(bool val) override;

private:
  std::string TAG;

  void log_err(const std::string &message);
  void log_warn(const std::string &message);
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
  int httpLogo(const std::string &request_path, HttpRequest* req, HttpResponse* resp);

  std::shared_ptr<hv::HttpService> m_pHttpService;
  std::string m_sApiPathPrefix;

  std::atomic<bool> m_metrics_enabled;

  // TODO refactoring it
  std::string m_logo_prefix;
  int m_logo_prefix_length;

  std::string m_sIndexHtml;
  std::string m_sScoreboardHtmlFolder;
  std::string m_sCacheResponseGameJson;
  std::string m_sCacheResponseTeamsJson;

  ctf01d::config *m_config;
};

static std::string prometheusEscapeLabelValue(const std::string &sValue) {
  std::string sResult;
  for (char c : sValue) {
    switch (c) {
      case '\\': sResult += "\\\\"; break;
      case '"': sResult += "\\\""; break;
      case '\n': sResult += "\\n"; break;
      default: sResult += c;
    }
  }
  return sResult;
}

static std::string prometheusLabels(std::initializer_list<std::pair<const char*, std::string>> labels) {
  std::ostringstream oss;
  oss << "{";
  bool bFirst = true;
  for (auto &label : labels) {
    if (!bFirst) {
      oss << ",";
    }
    oss << label.first << "=\"" << prometheusEscapeLabelValue(label.second) << "\"";
    bFirst = false;
  }
  oss << "}";
  return oss.str();
}

static void prometheusMetricInfo(std::ostringstream &oss, const std::string &sName, const std::string &sType, const std::string &sHelp) {
  oss << "# HELP " << sName << " " << sHelp << "\n"
    << "# TYPE " << sName << " " << sType << "\n";
}

// Decide whether a client may access /api/v1/metrics. The allowlist is a
// comma-separated list of IP patterns matched with libhv's wildcard matcher
// (only a trailing '*' is supported), e.g. "10.10.100.*, 127.0.*". On A/D CTF
// the team game network is usually private, so private ranges are NOT allowed
// implicitly — list only your monitoring/Docker subnet, not the game network.
static bool isMetricsClientAllowed(const std::string &sClientIp, const std::string &sAllowedList) {
  if (sClientIp == "::1") { // IPv6 loopback
    return true;
  }
  // Strip IPv4-mapped IPv6 prefix that libhv may report.
  std::string sIp = sClientIp;
  const std::string sMappedPrefix = "::ffff:";
  if (sIp.rfind(sMappedPrefix, 0) == 0) {
    sIp = sIp.substr(sMappedPrefix.size());
  }

  std::vector<std::string> vEntries = WsjcppCore::split(sAllowedList, ",");
  for (std::string sEntry : vEntries) {
    sEntry = WsjcppCore::trim(sEntry);
    if (sEntry.empty()) {
      continue;
    }
    if (hv_wildcard_match(sIp.c_str(), sEntry.c_str())) {
      return true;
    }
  }
  return false;
}

REGISTRY_WSJCPP_EMPLOY(employ_web_server)

employ_web_server::employ_web_server()
: WsjcppEmployBase({ ctf01d::web_server::name() }, { ctf01d::config::name(), ctf01d::activities::name() }) {
  TAG = ctf01d::web_server::name();
  m_sApiPathPrefix = "/api/v1/";
  
  // TODO refactoring it
  m_logo_prefix = "/logo/";
  m_logo_prefix_length = m_logo_prefix.size();
  
  m_config = findWsjcppEmploy<ctf01d::config>();
}

bool employ_web_server::init(const std::string &name, bool bSilent) {
  ctf01d::log::info(TAG, "init");
  m_metrics_enabled.store(m_config->scoreboard_metrics_enabled()->value());
  return true;
}

bool employ_web_server::deinit(const std::string &name, bool bSilent) {
  ctf01d::log::info(TAG, "deinit");
  return true;
}

static std::shared_ptr<ctf01d::logger> g_http_logger = std::shared_ptr<ctf01d::logger>(ctf01d::logger::create());

void EmployWebServer_custom_logger(int level, const char *msg, int len) {
  static const std::string TAG = "http-hv";
  std::string message(msg, len - 1); // remove last '\n' character
  switch (level) {
  case LOG_LEVEL_DEBUG:
    g_http_logger->info(TAG, "debug: " + message);
    break;
  case LOG_LEVEL_INFO:
    g_http_logger->info(TAG, message);
    break;
  case LOG_LEVEL_WARN:
    ctf01d::log::warn(TAG, message);
    g_http_logger->warn(TAG, message);
    break;
  case LOG_LEVEL_ERROR:
    ctf01d::log::err(TAG, message);
    g_http_logger->err(TAG, message);
    break;
  case LOG_LEVEL_FATAL:
    ctf01d::log::err(TAG, message);
    g_http_logger->throw_err(TAG, message);
    break;
  default:
    ctf01d::log::err(TAG, "Unknow level: " + message);
    g_http_logger->err(TAG, message);
  }
}

int employ_web_server::start() {

  auto pEmployConfig = findWsjcppEmploy<ctf01d::config>();
  g_http_logger->set_log_filename_prefix("http_hv");
  g_http_logger->set_log_dirpath(ctf01d::log::get_log_dirpath());
  g_http_logger->set_rotation_period_in_seconds(ctf01d::log::get_rotation_period_in_seconds());
  g_http_logger->set_enable_log_file(true);
  g_http_logger->set_enable_console_output(false);

  m_sScoreboardHtmlFolder = pEmployConfig->scoreboard_html_folder();
  updateJsonCache();

  std::string starting_message = "Starting scoreboard on http://localhost:" + std::to_string(pEmployConfig->scoreboard_port()) + "/";
  g_http_logger->ok(TAG, starting_message);
  ctf01d::log::ok(TAG, starting_message);

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
  m_pHttpService->GET("*", std::bind(&employ_web_server::httpWebFolder, this, std::placeholders::_1, std::placeholders::_2));

  hv::HttpServer server(m_pHttpService.get());
  server.setPort(pEmployConfig->scoreboard_port());
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

void employ_web_server::set_metrics_enabled(bool val) {
  m_metrics_enabled.store(val);
};

void employ_web_server::log_err(const std::string &message) {
  ctf01d::log::err(TAG, message);
  g_http_logger->err(TAG, message);
}

void employ_web_server::log_warn(const std::string &message) {
  ctf01d::log::warn(TAG, message);
  g_http_logger->warn(TAG, message);
}

void employ_web_server::updateJsonCache() {
  nlohmann::json jsonGame;
  nlohmann::json jsonTeams;

  jsonGame["game_id"] = m_config->game_id();
  jsonGame["game_name"] = m_config->game_name();
  jsonGame["game_start"] = WsjcppCore::formatTimeUTC(m_config->game_start_utc_in_seconds()) + " (UTC)";
  jsonGame["game_end"] = WsjcppCore::formatTimeUTC(m_config->game_end_utc_in_seconds()) + " (UTC)";
  jsonGame["game_has_coffee_break"] = m_config->game_has_coffee_break();
  jsonGame["game_coffee_break_start"] = WsjcppCore::formatTimeUTC(m_config->game_coffee_break_start_utc_in_seconds()) + " (UTC)";
  jsonGame["game_coffee_break_end"] = WsjcppCore::formatTimeUTC(m_config->game_coffee_break_end_utc_in_seconds()) + " (UTC)";
  jsonGame["teams"] = nlohmann::json::array();
  jsonGame["services"] = nlohmann::json::array();

  for (unsigned int i = 0; i < m_config->services().size(); i++) {
    ctf01d::service_config service_config = m_config->services()[i];
    if (service_config.is_enabled()) {
      nlohmann::json service_info;
      service_info["id"] = service_config.id();
      service_info["name"] = service_config.name();
      service_info["round_time_in_sec"] = service_config.round_in_seconds();
      jsonGame["services"].push_back(service_info);
    }
  }

  for (unsigned int i = 0; i < m_config->teams().size(); i++) {
      ctf01d::team_config teamConf = m_config->teams()[i];
      nlohmann::json teamInfo;
      teamInfo["id"] = teamConf.id();
      teamInfo["name"] = teamConf.name();
      teamInfo["ip_address"] = teamConf.ip_or_host();
      teamInfo["logo"] = "./logo/team/" + teamConf.id();
      teamInfo["logo-big"] = "./logo/big/team/" + teamConf.id();
      teamInfo["logo_last_write_time"] = teamConf.get_logo_last_modified_time();

      jsonGame["teams"].push_back(teamInfo);
      jsonTeams["teams"].push_back(teamInfo);
  }

  m_sCacheResponseGameJson = jsonGame.dump();
  m_sCacheResponseTeamsJson = jsonTeams.dump();
}

int employ_web_server::httpWebFolder(HttpRequest* req, HttpResponse* resp) {
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

  // hlogi("request_path = " + request_path);
  if (request_path == "/flag") { // Public endpoint. Allowed without authorization.
    return this->httpApiV1Flag(req, resp);
  }

  if (request_path.rfind(m_logo_prefix, 0) == 0) {
    return httpLogo(request_path, req, resp);
  }

  if (request_path.rfind(m_sApiPathPrefix, 0) == 0) {
    if (request_path == "/api/v1/game") { // Public endpoint. Allowed without authorization.
      return this->httpApiV1Game(req, resp);
    } else if (request_path == "/api/v1/game/current-time") { // Public endpoint. Allowed without authorization.
      return this->httpApiGameCurrentTime(req, resp);
    } else if (request_path == "/api/v1/scoreboard") { // Public endpoint. Allowed without authorization.
      return this->httpApiV1Scoreboard(req, resp);
    } else if (request_path == "/api/v1/my-ip") { // it's ok. Because network game is public space. This endpoint need for automatic configuration network.
      return this->httpApiV1MyIp(req, resp);
    } else if (request_path == "/api/v1/teams") { // Public endpoint. Allowed without authorization.
      return this->httpApiV1Teams(req, resp);
    } else if (request_path == "/api/v1/metrics" && m_metrics_enabled.load()) { // Config-gated: disabled by default + IP allowlist (see httpApiV1Metrics).
      return this->httpApiV1Metrics(req, resp);
    }
    return 404;
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

int employ_web_server::httpApiV1Game(HttpRequest* req, HttpResponse* resp) {
  // std::cout << m_sCacheResponseGameJson << std::endl;
  resp->Data(
    (void *)(m_sCacheResponseGameJson.c_str()),
    m_sCacheResponseGameJson.length(),
    true // nocopy
  );
  resp->SetContentTypeByFilename("game.json");
  return 200;
}

int employ_web_server::httpApiGameCurrentTime(HttpRequest* req, HttpResponse* resp) {
  // TODO maybe need optimization keep json response for every second.
  auto now = std::chrono::system_clock::now().time_since_epoch();
  int nCurrentTimeSec = std::chrono::duration_cast<std::chrono::seconds>(now).count();
  nlohmann::json current_time;
  current_time["current-time"] = nCurrentTimeSec - m_config->game_start_utc_in_seconds();
  std::string json_str = current_time.dump();
  resp->Data(
      (void *)(json_str.c_str()),
      json_str.length(),
      false // force copy
  );
  resp->SetContentTypeByFilename("current-time.json");
  return 200;
}

int employ_web_server::httpApiV1Teams(HttpRequest* req, HttpResponse* resp) {
  resp->Data(
    (void *)(m_sCacheResponseTeamsJson.c_str()),
    m_sCacheResponseTeamsJson.length(),
    true // nocopy
  );
  resp->SetContentTypeByFilename("teams.json");
  return 200;
}

int employ_web_server::httpApiV1MyIp(HttpRequest* req, HttpResponse* resp) {
  resp->json["my-ip"] = req->client_addr.ip;
  return 200;
}

int employ_web_server::httpApiV1Scoreboard(HttpRequest* req, HttpResponse* resp) {
  auto teamLogos = findWsjcppEmploy<ctf01d::images>();
  teamLogos->update_last_change_time();

  nlohmann::json jsonScoreboard = m_config->scoreboard()->to_json();
  teamLogos->update_scoreboard_json(jsonScoreboard);
  std::string sScoreboardJson = jsonScoreboard.dump();
  resp->Data(
      (void *)(sScoreboardJson.c_str()),
      sScoreboardJson.length(),
      false // nocopy - force copy
  );
  resp->SetContentTypeByFilename("scoreboard.json");
  return 200;
}

int employ_web_server::httpApiV1GetPaths(HttpRequest* req, HttpResponse* resp) {
  // TODO
  return resp->Json(m_pHttpService->Paths());
}

int employ_web_server::httpApiV1Flag(HttpRequest* req, HttpResponse* resp) {
  auto now = std::chrono::system_clock::now().time_since_epoch();
  int nCurrentTimeSec = std::chrono::duration_cast<std::chrono::seconds>(now).count();
  std::string request_ip = req->client_addr.ip;
  std::string sRequestIP_MsgSuffix = " (" + request_ip + ")";

  if (nCurrentTimeSec < m_config->game_start_utc_in_seconds()) {
    const std::string sErrorMsg = " Error(-8): Game not started yet";
    log_err(sRequestIP_MsgSuffix + sErrorMsg);
    resp->String(sErrorMsg);
    return 400;
  }

  if (m_config->game_has_coffee_break()
    && nCurrentTimeSec > m_config->game_coffee_break_start_utc_in_seconds()
    && nCurrentTimeSec < m_config->game_coffee_break_end_utc_in_seconds()
  ) {
    static const std::string sErrorMsg = "Error(-8): Game on coffee break now";
    log_err(sErrorMsg + sRequestIP_MsgSuffix);
    resp->String(sErrorMsg);
    return 400;
  }

  if (nCurrentTimeSec > m_config->game_end_utc_in_seconds()) {
    static const std::string sErrorMsg = "Error(-9): Game already ended";
    log_warn(sErrorMsg + sRequestIP_MsgSuffix);
    resp->String(sErrorMsg);
    return 400;
  }

  std::string sTeamId = req->GetParam("team_id");
  sTeamId = WsjcppCore::trim(sTeamId);
  sTeamId = WsjcppCore::toLower(sTeamId);
  std::string sFlag = req->GetParam("flag");
  sFlag = WsjcppCore::trim(sFlag);
  sFlag = WsjcppCore::toLower(sFlag);

  // todo if enabled detect-team-by-subnet > automatically detect team_id
  if (sTeamId == "") {
    // TODO server statistics
    static const std::string sErrorMsg = "Error(-10): Not found get-parameter 'team_id' or parameter is empty";
    log_err(sErrorMsg + sRequestIP_MsgSuffix);
    resp->String(sErrorMsg);
    return 400;
  }

  if (sFlag == "") {
    // TODO server statistics
    static const std::string sErrorMsg = "Error(-11): Not found get-parameter 'flag' or parameter is empty";
    log_err(sErrorMsg + sRequestIP_MsgSuffix);
    resp->String(sErrorMsg);
    return 400;
  }

  // TODO optimize
  bool bTeamFound = false;
  for (unsigned int i_team = 0; i_team < m_config->teams().size(); i_team++) {
    ctf01d::team_config teamConf = m_config->teams()[i_team];
    if (teamConf.id() == sTeamId) {
      bTeamFound = true;
    }
  }

  if (!bTeamFound) {
    // TODO server statistics
    static const std::string sErrorMsg = "Error(-130): this is team not found";
    log_err(sErrorMsg + sRequestIP_MsgSuffix);
    resp->String(sErrorMsg);
    return 400;
  }

  // TODO test speed of regexp and if will be simple compare every symbol.
  const static std::regex reFlagFormat("c01d[a-f0-9]{4,4}-[a-f0-9]{4,4}-[a-f0-9]{4,4}-[a-f0-9]{4,4}-[a-f0-9]{4,4}[0-9]{8,8}");
  if (!std::regex_match(sFlag, reFlagFormat)) {
    // TODO server statistics
    static const std::string sErrorMsg = "Error(-140): flag has wrong format";
    log_err(sErrorMsg + sRequestIP_MsgSuffix);
    resp->String(sErrorMsg);
    return 400;
  }
  
  m_config->scoreboard()->insert_flag_attempt(sTeamId, sFlag, request_ip);

  ctf01d::flag flag;
  if (!findWsjcppEmploy<ctf01d::alive_flags>()->find_alive_flag(sFlag, flag)) {
    // TODO server statistics
    static const std::string sErrorMsg = "Error(-150): flag is too old or flag never existed or flag already stole.";
    g_http_logger->info(TAG, sErrorMsg + ". Received flag {" + sFlag + "} from {" + sTeamId + "}" + sRequestIP_MsgSuffix);
    resp->String(sErrorMsg);
    return 403;
  }

  long nCurrentTimeMSec = (long)nCurrentTimeSec;
  nCurrentTimeMSec = nCurrentTimeMSec*1000;

  if (flag.getTimeEndInMs() < nCurrentTimeMSec) {
    // TODO server statistics
    static const std::string sErrorMsg = "Error(-151): flag is too old";
    log_err(sErrorMsg + ". Received flag {" + sFlag + "} from {" + sTeamId + "}" + sRequestIP_MsgSuffix);
    resp->String(sErrorMsg);
    return 403;
  }

  // if (flag.teamStole() == sTeamId) {
  //   response.forbidden().sendText("Error(-160): flag already stole by your team");
  //   log_err("Error(-160): Received flag {" + sFlag + "} from {" + sTeamId + "} (flag already stole by your team)");
  //   return true;
  // }

  if (flag.getTeamId() == sTeamId) {
    // TODO server statistics
    static const std::string sErrorMsg = "Error(-180): this is your flag";
    log_err(sErrorMsg + ". Received flag {" + sFlag + "} from {" + sTeamId + "}" + sRequestIP_MsgSuffix);
    resp->String(sErrorMsg);
    return 403;
  }

  std::string sServiceStatus = m_config->scoreboard()->service_status(sTeamId, flag.getServiceId());

  // std::cout << "sServiceStatus: " << sServiceStatus << "\n";

  if (sServiceStatus != ctf01d::service_status_cell::SERVICE_UP) {
    // TODO server statistics
    static const std::string sErrorMsg = "Error(-190): Your same service is dead. Try later.";
    log_err(sErrorMsg + ". Received flag {" + sFlag + "} from {" + sTeamId + "}" + sRequestIP_MsgSuffix);
    resp->String(sErrorMsg);
    return 403;
  }

  // TODO light update scoreboard
  // incrementAttackScore performs the dedup check under its own mutex,
  // so check-then-insert is atomic against concurrent submissions.
  std::optional<int> oPoints = m_config->scoreboard()->increment_attack_score(flag, sTeamId);
  if (!oPoints.has_value()) {
    // TODO server statistics
    static const std::string sErrorMsg = "Error(-170): flag already stolen by your team";
    log_err(sErrorMsg + ". Received flag {" + sFlag + "} from {" + sTeamId + "}" + sRequestIP_MsgSuffix);
    resp->String(sErrorMsg);
    return 403;
  }
  std::string sPoints = std::to_string(oPoints.value());

  std::string sResponse = "Accepted: Received flag {" + sFlag + "} from {" + sTeamId + "} (Accepted + " + sPoints + ")";
  // really need send to current ???
  g_http_logger->ok(TAG, sResponse + sRequestIP_MsgSuffix);
  ctf01d::log::ok(TAG, sResponse + sRequestIP_MsgSuffix);
  resp->Data(
      (void *)(sResponse.c_str()),
      sResponse.size(),
      false // copy buffer
  );
  resp->content_type = TEXT_PLAIN;
  return 200;
}

int employ_web_server::httpLogo(const std::string &request_path, HttpRequest* req, HttpResponse* resp) {
  // TODO refactoring it

  std::string id = request_path.substr(m_logo_prefix_length, request_path.length() - m_logo_prefix_length);
  auto images = findWsjcppEmploy<ctf01d::images>();
  std::shared_ptr<ctf01d::image> img = images->find_image(id);

  if (!img) {
    return 404;
  }
  resp->Data(
    img->pBuffer,
    img->nBufferSize,
    true // nocopy
  );
  resp->SetContentTypeByFilename(img->filename().c_str());
  return 200;

}

int employ_web_server::httpApiV1Metrics(HttpRequest* req, HttpResponse* resp) {

  if (!isMetricsClientAllowed(req->client_addr.ip, m_config->scoreboard_metrics_allowed_for()->value())) {
    resp->String("Forbidden");
    return 403;
  }

  nlohmann::json jsonScoreboard = m_config->scoreboard()->to_json();

  std::ostringstream oss;

  prometheusMetricInfo(oss, "ctf01d_build_info", "gauge", "ctf01d build information.");
  oss << "ctf01d_build_info" << prometheusLabels({{"version", std::string(WSJCPP_APP_VERSION)}}) << " 1\n";

  prometheusMetricInfo(oss, "ctf01d_game_start_timestamp_seconds", "gauge", "Game start (UTC).");
  oss << "ctf01d_game_start_timestamp_seconds " << jsonScoreboard["game"]["t0"].get<long>() << "\n";
  prometheusMetricInfo(oss, "ctf01d_game_end_timestamp_seconds", "gauge", "Game end (UTC).");
  oss << "ctf01d_game_end_timestamp_seconds " << jsonScoreboard["game"]["t3"].get<long>() << "\n";
  prometheusMetricInfo(oss, "ctf01d_game_coffee_break_start_timestamp_seconds", "gauge", "Coffee break start.");
  oss << "ctf01d_game_coffee_break_start_timestamp_seconds " << jsonScoreboard["game"]["t1"].get<long>() << "\n";
  prometheusMetricInfo(oss, "ctf01d_game_coffee_break_end_timestamp_seconds", "gauge", "Coffee break end.");
  oss << "ctf01d_game_coffee_break_end_timestamp_seconds " << jsonScoreboard["game"]["t2"].get<long>() << "\n";
  prometheusMetricInfo(oss, "ctf01d_game_current_time_seconds", "gauge", "Server current time.");
  oss << "ctf01d_game_current_time_seconds " << jsonScoreboard["game"]["tc"].get<long>() << "\n";

  prometheusMetricInfo(oss, "ctf01d_teams_total", "gauge", "Teams in the game.");
  oss << "ctf01d_teams_total " << jsonScoreboard["scoreboard"].size() << "\n";
  prometheusMetricInfo(oss, "ctf01d_services_total", "gauge", "Services in the game.");
  oss << "ctf01d_services_total " << jsonScoreboard["s_sta"].size() << "\n";
  prometheusMetricInfo(oss, "ctf01d_flag_attempts_total", "counter", "Total flag submission attempts.");
  oss << "ctf01d_flag_attempts_total " << jsonScoreboard["sum_act"].get<long>() << "\n";
  prometheusMetricInfo(oss, "ctf01d_flags_live", "gauge", "Currently active flags.");
  oss << "ctf01d_flags_live " << findWsjcppEmploy<ctf01d::alive_flags>()->count_alive_flags() << "\n";

  prometheusMetricInfo(oss, "ctf01d_team_score", "gauge", "Team score.");
  for (auto &it : jsonScoreboard["scoreboard"].items()) {
    oss << "ctf01d_team_score" << prometheusLabels({{"team", it.key()}})
      << " " << it.value()["points"].get<double>() << "\n";
  }
  prometheusMetricInfo(oss, "ctf01d_team_place", "gauge", "Team current place (1 = leader).");
  for (auto &it : jsonScoreboard["scoreboard"].items()) {
    oss << "ctf01d_team_place" << prometheusLabels({{"team", it.key()}})
      << " " << it.value()["place"].get<int>() << "\n";
  }
  prometheusMetricInfo(oss, "ctf01d_team_tries_total", "counter", "Team flag submission attempts.");
  for (auto &it : jsonScoreboard["scoreboard"].items()) {
    oss << "ctf01d_team_tries_total" << prometheusLabels({{"team", it.key()}})
      << " " << it.value()["tries"].get<long>() << "\n";
  }

  static const std::vector<std::string> vStatuses = {
    ctf01d::service_status_cell::SERVICE_UP,
    ctf01d::service_status_cell::SERVICE_DOWN,
    ctf01d::service_status_cell::SERVICE_MUMBLE,
    ctf01d::service_status_cell::SERVICE_CORRUPT,
    ctf01d::service_status_cell::SERVICE_SHIT,
    ctf01d::service_status_cell::SERVICE_WAIT,
    ctf01d::service_status_cell::SERVICE_COFFEE_BREAK
  };

  prometheusMetricInfo(oss, "ctf01d_team_service_status", "gauge", "Service status as one-hot: 1 if current, else 0.");
  for (auto &team : jsonScoreboard["scoreboard"].items()) {
    for (auto &service : team.value()["ts_sta"].items()) {
      const std::string sCurrentStatus = service.value()["status"].get<std::string>();
      for (auto &sStatus : vStatuses) {
        oss << "ctf01d_team_service_status"
          << prometheusLabels({{"team", team.key()}, {"service", service.key()}, {"status", sStatus}})
          << " " << (sCurrentStatus == sStatus ? 1 : 0) << "\n";
      }
    }
  }

  prometheusMetricInfo(oss, "ctf01d_team_service_sla_percent", "gauge", "SLA percent (0..100).");
  for (auto &team : jsonScoreboard["scoreboard"].items()) {
    for (auto &service : team.value()["ts_sta"].items()) {
      oss << "ctf01d_team_service_sla_percent"
        << prometheusLabels({{"team", team.key()}, {"service", service.key()}})
        << " " << service.value()["sla"].get<double>() << "\n";
    }
  }

  prometheusMetricInfo(oss, "ctf01d_team_service_defense_flags_total", "counter", "Team defended flags per service.");
  for (auto &team : jsonScoreboard["scoreboard"].items()) {
    for (auto &service : team.value()["ts_sta"].items()) {
      oss << "ctf01d_team_service_defense_flags_total"
        << prometheusLabels({{"team", team.key()}, {"service", service.key()}})
        << " " << service.value()["def"].get<long>() << "\n";
    }
  }

  prometheusMetricInfo(oss, "ctf01d_team_service_defense_points", "gauge", "Team defense points per service.");
  for (auto &team : jsonScoreboard["scoreboard"].items()) {
    for (auto &service : team.value()["ts_sta"].items()) {
      oss << "ctf01d_team_service_defense_points"
        << prometheusLabels({{"team", team.key()}, {"service", service.key()}})
        << " " << service.value()["pt_def"].get<double>() << "\n";
    }
  }

  prometheusMetricInfo(oss, "ctf01d_team_service_attack_flags_total", "counter", "Team stolen flags per service.");
  for (auto &team : jsonScoreboard["scoreboard"].items()) {
    for (auto &service : team.value()["ts_sta"].items()) {
      oss << "ctf01d_team_service_attack_flags_total"
        << prometheusLabels({{"team", team.key()}, {"service", service.key()}})
        << " " << service.value()["att"].get<long>() << "\n";
    }
  }

  prometheusMetricInfo(oss, "ctf01d_team_service_attack_points", "gauge", "Team attack points per service.");
  for (auto &team : jsonScoreboard["scoreboard"].items()) {
    for (auto &service : team.value()["ts_sta"].items()) {
      oss << "ctf01d_team_service_attack_points"
        << prometheusLabels({{"team", team.key()}, {"service", service.key()}})
        << " " << service.value()["pt_att"].get<double>() << "\n";
    }
  }

  prometheusMetricInfo(oss, "ctf01d_service_attack_flags_total", "counter", "Total stolen flags per service (across teams).");
  for (auto &it : jsonScoreboard["s_sta"].items()) {
    oss << "ctf01d_service_attack_flags_total" << prometheusLabels({{"service", it.key()}})
      << " " << it.value()["af_att"].get<long>() << "\n";
  }

  prometheusMetricInfo(oss, "ctf01d_service_defense_flags_total", "counter", "Total defended flags per service.");
  for (auto &it : jsonScoreboard["s_sta"].items()) {
    oss << "ctf01d_service_defense_flags_total" << prometheusLabels({{"service", it.key()}})
      << " " << it.value()["af_def"].get<long>() << "\n";
  }

  prometheusMetricInfo(oss, "ctf01d_service_first_blood_timestamp_seconds", "gauge", "First blood UTC seconds; 0 if none yet.");
  for (auto &it : jsonScoreboard["s_sta"].items()) {
    const std::string sFirstBloodTeamId = it.value()["first_blood"].get<std::string>();
    if (sFirstBloodTeamId.empty() || sFirstBloodTeamId == "?") {
      oss << "ctf01d_service_first_blood_timestamp_seconds"
        << prometheusLabels({{"service", it.key()}, {"team", ""}})
        << " 0\n";
    } else {
      oss << "ctf01d_service_first_blood_timestamp_seconds"
        << prometheusLabels({{"service", it.key()}, {"team", sFirstBloodTeamId}})
        << " " << it.value()["first_blood_ts"].get<long>() << "\n";
    }
  }

  std::string sResponse = oss.str();
  resp->Data(
    (void *)(sResponse.c_str()),
    sResponse.size(),
    false // copy buffer
  );
  resp->content_type = TEXT_PLAIN;
  resp->headers["Content-Type"] = "text/plain; version=0.0.4; charset=utf-8";
  return 200;
}
