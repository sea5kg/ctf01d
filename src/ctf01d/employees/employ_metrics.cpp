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

#include <wsjcpp_employees.h>
#include <json.hpp>
#include "ctf01d/include/ctf01d_images.h"
#include "ctf01d/objects/ctf01d_image.h"
#include <wsjcpp_core.h>
#include <filesystem>
#include "ctf01d/include/ctf01d_config.h"
#include "ctf01d/include/ctf01d_globals.h"
#include "ctf01d/include/ctf01d_scoreboard.h"
#include "ctf01d/include/ctf01d_metrics.h"
#include "ctf01d/include/ctf01d_alive_flags.h"
#include "ctf01d/objects/ctf01d_service_status_cell.h"
#include <sea5kg_logger.h>
#include "hbase.h"  // libhv: hv_wildcard_match


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

class employ_metrics : public WsjcppEmployBase, public ctf01d::metrics, public ctf01d::listener_config_changed {
public:
  employ_metrics();
  virtual bool init(const std::string &name, bool silent) override;
  virtual bool deinit(const std::string &name, bool silent) override;

  // ctf01d::listener_config_changed
  virtual void config_changed() override;

  // ctf01d::metrics
  virtual bool is_prometheus_metrics_client_allowed(const std::string &request_ip) override;
  virtual std::string prometheus_metrics() override;

private:
  std::string TAG;
  std::string m_prometheus_metrics_allowed_list_ip;
  bool m_prometheus_metrics_enabled = false;
  ctf01d::config *m_config;
  ctf01d::scoreboard *m_scoreboard;
  std::mutex m_mutex;
};

REGISTRY_WSJCPP_EMPLOY(employ_metrics)

employ_metrics::employ_metrics()
: WsjcppEmployBase({ ctf01d::metrics::name() }, { ctf01d::config::name(), ctf01d::scoreboard::name(), ctf01d::alive_flags::name() }) {
  TAG = ctf01d::metrics::name();
  m_scoreboard = nullptr;
  m_config = nullptr;
  m_prometheus_metrics_allowed_list_ip = "";
  m_prometheus_metrics_enabled = false;
}

bool employ_metrics::init(const std::string &name, bool silent) {
  sea5kg::log::info(TAG, "init");
  m_scoreboard = findWsjcppEmploy<ctf01d::scoreboard>();
  m_config = findWsjcppEmploy<ctf01d::config>();
  config_changed();
  m_config->add_listener(this);
  return true;
}

bool employ_metrics::deinit(const std::string &name, bool silent) {
  sea5kg::log::info(TAG, "deinit");
  return true;
}

// ctf01d::listener_config_changed
void employ_metrics::config_changed() {
  if (m_config == nullptr) {
    return;
  }
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_prometheus_metrics_allowed_list_ip != m_config->scoreboard_metrics_allowed_for()) {
    sea5kg::log::warn(TAG, "Applied option: '" + m_prometheus_metrics_allowed_list_ip + "' -> '" + m_config->scoreboard_metrics_allowed_for() + "'");
    m_prometheus_metrics_allowed_list_ip = m_config->scoreboard_metrics_allowed_for();
  }
  if (m_prometheus_metrics_enabled != m_config->scoreboard_metrics_enabled()) {
    sea5kg::log::warn(TAG, "Applied option: prometheus metric enabled '" + std::string(m_prometheus_metrics_enabled ? "yes" : "no") + "' -> '" + (m_config->scoreboard_metrics_enabled() ? "yes" : "no") + "'");
    m_prometheus_metrics_enabled = m_config->scoreboard_metrics_enabled();
  }
}

bool employ_metrics::is_prometheus_metrics_client_allowed(const std::string &request_ip) {
  std::lock_guard<std::mutex> lock(m_mutex);
  // Decide whether a client may access /api/v1/metrics. The allowlist is a
  // comma-separated list of IP patterns matched with libhv's wildcard matcher
  // (only a trailing '*' is supported), e.g. "10.10.100.*, 127.0.*". On A/D CTF
  // the team game network is usually private, so private ranges are NOT allowed
  // implicitly — list only your monitoring/Docker subnet, not the game network.
  if (request_ip == "::1") { // IPv6 loopback
    return true;
  }
  // Strip IPv4-mapped IPv6 prefix that libhv may report.
  std::string sIp = request_ip;
  const std::string sMappedPrefix = "::ffff:";
  if (sIp.rfind(sMappedPrefix, 0) == 0) {
    sIp = sIp.substr(sMappedPrefix.size());
  }

  std::vector<std::string> vEntries = wsjcpp::split(m_prometheus_metrics_allowed_list_ip, ",");
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

std::string employ_metrics::prometheus_metrics() {
  std::lock_guard<std::mutex> lock(m_mutex);

  nlohmann::json jsonScoreboard = m_scoreboard->to_json();

  std::ostringstream oss;

  prometheusMetricInfo(oss, "ctf01d_build_info", "gauge", "ctf01d build information.");
  oss << "ctf01d_build_info" << prometheusLabels({{"version", m_config->ctf01d_version()}}) << " 1\n";

  prometheusMetricInfo(oss, "ctf01d_game_start_timestamp_seconds", "gauge", "Game start (UTC).");
  oss << "ctf01d_game_start_timestamp_seconds " << jsonScoreboard["game"]["t0"].get<long>() << "\n";
  prometheusMetricInfo(oss, "ctf01d_game_end_timestamp_seconds", "gauge", "Game end (UTC).");
  oss << "ctf01d_game_end_timestamp_seconds " << jsonScoreboard["game"]["t3"].get<long>() << "\n";
  prometheusMetricInfo(oss, "ctf01d_game_coffee_break_start_timestamp_seconds", "gauge", "Coffee break start.");
  oss << "ctf01d_game_coffee_break_start_timestamp_seconds " << jsonScoreboard["game"]["t1"].get<long>() << "\n";
  prometheusMetricInfo(oss, "ctf01d_game_coffee_break_end_timestamp_seconds", "gauge", "Coffee break end.");
  oss << "ctf01d_game_coffee_break_end_timestamp_seconds " << jsonScoreboard["game"]["t2"].get<long>() << "\n";
  prometheusMetricInfo(oss, "ctf01d_game_current_time_seconds", "gauge", "Server current time.");
  oss << "ctf01d_game_current_time_seconds " << jsonScoreboard[ctf01d::json_fields::CURRENT_TIME].get<long>() << "\n";

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

  std::string ret = oss.str();
  return ret;
}
