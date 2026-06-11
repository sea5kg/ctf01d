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

#include "employ_scoreboard.h"
#include <wsjcpp_core.h>
#include "ctf01d/employees/employ_config.h"
#include "ctf01d/utils/ctf01d_logger.h"
#include <cmath>
#include <stdio.h>
#include <string>
#include <map>
#include <mutex>
#include <vector>

REGISTRY_WSJCPP_EMPLOY(EmployScoreboard)

EmployScoreboard::EmployScoreboard()
: WsjcppEmployBase({ EmployScoreboard::name() }, {}) {
  TAG = EmployScoreboard::name();
}

bool EmployScoreboard::init(const std::string &sName, bool bSilent) {
  if (!initServicesStats()) {
    return false;
  }
  return true;
}

bool EmployScoreboard::deinit(const std::string &sName, bool bSilent) {
  ctf01d::log::info(TAG, "deinit");
  return true;
}

bool EmployScoreboard::initServicesStats() {
  std::lock_guard<std::mutex> lock(m_mutex_services_stats);
  m_map_services_stats.clear();

  EmployConfig *pEmployConfig = findWsjcppEmploy<EmployConfig>();
  const std::vector<ctf01d::team_config> &teams_conf = pEmployConfig->teamsConf();
  const std::vector<ctf01d::service_config> &services_conf = pEmployConfig->servicesConf();

  // keep the list of the services ids
  for (unsigned int i = 0; i < services_conf.size(); i++) {
    std::string service_id = services_conf[i].id();
    m_map_services_stats[service_id] = std::make_shared<ctf01d::service_statistics>(service_id);
  }
  return true;
}

