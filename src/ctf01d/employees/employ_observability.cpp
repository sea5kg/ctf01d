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

#include "employ_observability.h"

#include <wsjcpp_core.h>
#include "ctf01d/utils/ctf01d_logger.h"

REGISTRY_WSJCPP_EMPLOY(EmployObservability)

EmployObservability::EmployObservability()
: WsjcppEmployBase({ EmployObservability::name() }, {}) {
  TAG = EmployObservability::name();
  m_nStartedAtSeconds = WsjcppCore::getCurrentTimeInSeconds();
}

bool EmployObservability::init(const std::string &sName, bool bSilent) {
  ctf01d::log::info(TAG, "init");
  return true;
}

bool EmployObservability::deinit(const std::string &sName, bool bSilent) {
  ctf01d::log::info(TAG, "deinit");
  return true;
}

long EmployObservability::started_at_seconds() {
  return m_nStartedAtSeconds;
}

void EmployObservability::register_checker_thread(const std::string &sTeamId, const std::string &sServiceId) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_checker_threads.insert(std::make_pair(sTeamId, sServiceId));
}

int EmployObservability::active_checker_threads() {
  std::lock_guard<std::mutex> lock(m_mutex);
  return static_cast<int>(m_checker_threads.size());
}

void EmployObservability::record_http_request(
  const std::string &sMethod,
  const std::string &sPath,
  int nCode,
  long nDurationMs
) {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto key = std::make_tuple(sMethod, sPath, nCode);
  auto &metric = m_http_requests[key];
  metric.method = sMethod;
  metric.path = sPath;
  metric.code = nCode;
  metric.count++;
  metric.duration_seconds_sum += double(nDurationMs) / 1000.0;
}

std::vector<ctf01d::http_observation> EmployObservability::http_requests() {
  std::vector<ctf01d::http_observation> vRet;
  std::lock_guard<std::mutex> lock(m_mutex);
  for (auto &it : m_http_requests) {
    vRet.push_back(it.second);
  }
  return vRet;
}

void EmployObservability::record_flag_submission(
  const std::string &sTeamId,
  const std::string &sResult,
  const std::string &sErrorCode
) {
  std::string sTeam = sTeamId.empty() ? "unknown" : sTeamId;

  std::lock_guard<std::mutex> lock(m_mutex);
  auto key = std::make_tuple(sTeam, sResult, sErrorCode);
  auto &metric = m_flag_submissions[key];
  metric.team = sTeam;
  metric.result = sResult;
  metric.error_code = sErrorCode;
  metric.count++;
}

std::vector<ctf01d::flag_submission_observation> EmployObservability::flag_submissions() {
  std::vector<ctf01d::flag_submission_observation> vRet;
  std::lock_guard<std::mutex> lock(m_mutex);
  for (auto &it : m_flag_submissions) {
    vRet.push_back(it.second);
  }
  return vRet;
}

void EmployObservability::record_checker_run(
  const std::string &sTeamId,
  const std::string &sServiceId,
  const std::string &sCommand,
  int nExitCode,
  const std::string &sResult,
  long nDurationMs
) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto key = std::make_tuple(sTeamId, sServiceId, sCommand, sResult, nExitCode);
  auto &run_metric = m_checker_runs[key];
  run_metric.team = sTeamId;
  run_metric.service = sServiceId;
  run_metric.command = sCommand;
  run_metric.result = sResult;
  run_metric.exit_code = nExitCode;
  run_metric.count++;

  auto state_key = std::make_tuple(sTeamId, sServiceId, sCommand);
  auto &state_metric = m_checker_states[state_key];
  state_metric.team = sTeamId;
  state_metric.service = sServiceId;
  state_metric.command = sCommand;
  state_metric.result = sResult;
  state_metric.exit_code = nExitCode;
  state_metric.last_duration_seconds = double(nDurationMs) / 1000.0;
  state_metric.last_run_timestamp_seconds = WsjcppCore::getCurrentTimeInSeconds();
  if (sResult == "up") {
    state_metric.consecutive_failures = 0;
  } else {
    state_metric.consecutive_failures++;
  }
}

std::vector<ctf01d::checker_run_observation> EmployObservability::checker_runs() {
  std::vector<ctf01d::checker_run_observation> vRet;
  std::lock_guard<std::mutex> lock(m_mutex);
  for (auto &it : m_checker_runs) {
    vRet.push_back(it.second);
  }
  return vRet;
}

std::vector<ctf01d::checker_state_observation> EmployObservability::checker_states() {
  std::vector<ctf01d::checker_state_observation> vRet;
  std::lock_guard<std::mutex> lock(m_mutex);
  for (auto &it : m_checker_states) {
    vRet.push_back(it.second);
  }
  return vRet;
}
