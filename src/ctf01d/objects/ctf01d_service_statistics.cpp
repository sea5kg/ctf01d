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

#include "ctf01d_service_statistics.h"

namespace ctf01d {

service_statistics::service_statistics(const std::string &sServiceId) {
  TAG = "service_statistics-" + sServiceId;
  m_sServiceId = sServiceId;
  m_nAllStolenFlagsForService = 0;
  m_nAllDefenseFlagsForService = 0;
  m_sFirstBloodTeamId = "?";
  m_nFirstBloodTimeInSeconds = 0;
}

int service_statistics::getAllStolenFlagsForService() {
  return m_nAllStolenFlagsForService;
}

void service_statistics::doIncrementStolenFlagsForService() {
  m_nAllStolenFlagsForService++;
}

void service_statistics::setStolenFlagsForService(int nStolenFlags) {
  m_nAllStolenFlagsForService = nStolenFlags;
}

int service_statistics::getAllDefenseFlagsForService() {
  return m_nAllDefenseFlagsForService;
}

void service_statistics::doIncrementDefenseFlagsForService() {
  m_nAllDefenseFlagsForService++;
}

void service_statistics::setDefenseFlagsForService(int nAllDefenseFlagsForService) {
  m_nAllDefenseFlagsForService = nAllDefenseFlagsForService;
}

std::string service_statistics::getFirstBloodTeamId() {
  return m_sFirstBloodTeamId;
}

long service_statistics::getFirstBloodTime() {
  return m_nFirstBloodTimeInSeconds;
}

void service_statistics::updateJsonServiceStatistics(nlohmann::json &jsonCosts) {
  jsonCosts["af_att"] = m_nAllStolenFlagsForService;
  jsonCosts["af_def"] = m_nAllDefenseFlagsForService;
  jsonCosts["first_blood"] = m_sFirstBloodTeamId;
  jsonCosts["first_blood_ts"] = m_nFirstBloodTimeInSeconds;
}

void service_statistics::setFirstBloodTeamId(const std::string &sFirstBlood, long nDateACtion) {
  m_sFirstBloodTeamId = sFirstBlood;
  m_nFirstBloodTimeInSeconds = nDateACtion / 1000;
}

} // namespace ctf01d