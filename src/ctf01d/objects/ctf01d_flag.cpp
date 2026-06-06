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

#include "ctf01d_flag.h"
#include "ctf01d/utils/ctf01d_logger.h"
#include <wsjcpp_core.h>
// #include <fstream>
#include <cstring>

namespace ctf01d {

flag::flag() {
  // flag id
  m_sId = "qweRT12345";
  // flag format
  m_sValue = "c01d0000-0000-0000-0000-000000000000";
  m_sTeamId = "";
  m_sServiceId = "";
  m_nTimeStartInMs = 0;
  m_nTimeEndInMs = 0;
}

void flag::generateRandomFlag(int nFlagLifetimeInSeconds, const std::string &sTeamId, const std::string &sServiceId, int nGameStartUTCInSec) {
  long nTimeStartInMs = WsjcppCore::getCurrentTimeInMilliseconds();
  long nTimeEndInMs = nTimeStartInMs + nFlagLifetimeInSeconds*1000;
  setTimeStartInMs(nTimeStartInMs);
  setTimeEndInMs(nTimeEndInMs);

  generateId();
  generateValue(nGameStartUTCInSec);
  m_sTeamId = sTeamId;
  m_sServiceId = sServiceId;
}

void ctf01d::flag::generateId() {
  static const std::string sAlphabet =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

  std::string sFlagId = "";
  int nLenId = m_sId.size();
  for (int i = 0; i < 10; ++i) {
    m_sId[i] = sAlphabet[rand() % sAlphabet.length()];
  }
}

void ctf01d::flag::setId(const std::string &sId) {
  m_sId = sId;
}

const std::string &ctf01d::flag::getId() const {
  return m_sId;
}

void ctf01d::flag::generateValue(int nGameStartUTCInSec) {
  // TODO redesign more freeble format
  static const std::string sAlphabet = "0123456789abcdef";
  char sUuid[37];
  memset(&sUuid, '\0', 37);
  sUuid[8] = '-';
  sUuid[13] = '-';
  sUuid[18] = '-';
  sUuid[23] = '-';

  for(int i = 4; i < 28; i++){
    if (i != 8 && i != 13 && i != 18 && i != 23) {
      m_sValue[i] = sAlphabet[rand() % sAlphabet.length()];
    }
  }

  // set timepoint
  int dt = m_nTimeStartInMs / 1000 - nGameStartUTCInSec;
  std::string sTimePoint = std::to_string(dt);
  int nTimePointLen = sTimePoint.size();
  if (nTimePointLen > 8) {
    ctf01d::log::throw_err("ctf01d::flag::generateValue", "Really game was started more then 3 years ago ??? got value: " + sTimePoint);
  }
  int nPos = m_sValue.size() - 1;
  for (int i = nTimePointLen - 1; i >= 0; i--) {
    m_sValue[nPos] = sTimePoint[i];
    nPos--;
  }
  // 03268167

  // std::cout << "sTimePoint: " << sTimePoint << "\n";
  // this->setValue(std::string(sUuid) + sTimePoint);
}

void ctf01d::flag::setValue(const std::string &sValue) {
  // TODO validate format
  // c01d...00000000 - prefix and time
  m_sValue = sValue;
}

const std::string &ctf01d::flag::getValue() const {
  return m_sValue;
}

void ctf01d::flag::setTeamId(const std::string &sTeamId) {
  m_sTeamId = sTeamId;
}

const std::string &ctf01d::flag::getTeamId() const {
  return m_sTeamId;
}

void ctf01d::flag::setServiceId(const std::string &sServiceId) {
  m_sServiceId = sServiceId;
}

const std::string &ctf01d::flag::getServiceId() const {
  return m_sServiceId;
}

void ctf01d::flag::setTimeStartInMs(long nTimeStartInMs) {
  m_nTimeStartInMs = nTimeStartInMs;
}

long ctf01d::flag::getTimeStartInMs() const {
  return m_nTimeStartInMs;
}

void ctf01d::flag::setTimeEndInMs(long nTimeEndInMs) {
  m_nTimeEndInMs = nTimeEndInMs;
}

long ctf01d::flag::getTimeEndInMs() const {
  return m_nTimeEndInMs;
}

void ctf01d::flag::copyFrom(const ctf01d::flag &flag) {
  this->setId(flag.getId());
  this->setValue(flag.getValue());
  this->setServiceId(flag.getServiceId());
  this->setTeamId(flag.getTeamId());
  this->setTimeStartInMs(flag.getTimeStartInMs());
  this->setTimeEndInMs(flag.getTimeEndInMs());
}

} // namespace ctf01d