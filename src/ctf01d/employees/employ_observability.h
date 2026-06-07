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

#include <map>
#include <mutex>
#include <set>
#include <string>
#include <tuple>
#include <vector>
#include <wsjcpp_employees.h>

namespace ctf01d {

struct http_observation {
  std::string method;
  std::string path;
  int code = 0;
  long count = 0;
  double duration_seconds_sum = 0.0;
};

struct flag_submission_observation {
  std::string team;
  std::string result;
  std::string error_code;
  long count = 0;
};

struct checker_run_observation {
  std::string team;
  std::string service;
  std::string command;
  std::string result;
  int exit_code = 0;
  long count = 0;
};

struct checker_state_observation {
  std::string team;
  std::string service;
  std::string command;
  std::string result;
  int exit_code = 0;
  double last_duration_seconds = 0.0;
  long last_run_timestamp_seconds = 0;
  long consecutive_failures = 0;
};

} // namespace ctf01d

class EmployObservability : public WsjcppEmployBase {
public:
  EmployObservability();
  static std::string name() { return "EmployObservability"; }
  virtual bool init(const std::string &sName, bool bSilent) override;
  virtual bool deinit(const std::string &sName, bool bSilent) override;

  long started_at_seconds();

  void register_checker_thread(const std::string &sTeamId, const std::string &sServiceId);
  int active_checker_threads();

  void record_http_request(
    const std::string &sMethod,
    const std::string &sPath,
    int nCode,
    long nDurationMs
  );
  std::vector<ctf01d::http_observation> http_requests();

  void record_flag_submission(
    const std::string &sTeamId,
    const std::string &sResult,
    const std::string &sErrorCode
  );
  std::vector<ctf01d::flag_submission_observation> flag_submissions();

  void record_checker_run(
    const std::string &sTeamId,
    const std::string &sServiceId,
    const std::string &sCommand,
    int nExitCode,
    const std::string &sResult,
    long nDurationMs
  );
  std::vector<ctf01d::checker_run_observation> checker_runs();
  std::vector<ctf01d::checker_state_observation> checker_states();

private:
  std::string TAG;
  long m_nStartedAtSeconds;
  std::mutex m_mutex;
  std::set<std::pair<std::string, std::string>> m_checker_threads;
  std::map<std::tuple<std::string, std::string, int>, ctf01d::http_observation> m_http_requests;
  std::map<std::tuple<std::string, std::string, std::string>, ctf01d::flag_submission_observation> m_flag_submissions;
  std::map<std::tuple<std::string, std::string, std::string, std::string, int>, ctf01d::checker_run_observation> m_checker_runs;
  std::map<std::tuple<std::string, std::string, std::string>, ctf01d::checker_state_observation> m_checker_states;
};
