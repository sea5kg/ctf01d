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

#include "ctf01d_service_status_cell.h"
#include <wsjcpp_core.h>


namespace ctf01d {

const std::string service_status_cell::SERVICE_UP = "up";
const std::string service_status_cell::SERVICE_DOWN = "down";
const std::string service_status_cell::SERVICE_MUMBLE = "mumble";
const std::string service_status_cell::SERVICE_CORRUPT = "corrupt";
const std::string service_status_cell::SERVICE_SHIT = "shit";
const std::string service_status_cell::SERVICE_WAIT = "wait";
const std::string service_status_cell::SERVICE_COFFEE_BREAK = "coffee-break";

service_status_cell::service_status_cell(const std::string &sServiceId) {
  m_nUpPointTimeInSec = WsjcppCore::getCurrentTimeInSeconds();
  TAG = "service_status_cell-" + sServiceId;
  m_sServiceId = sServiceId;
  m_sStatus = service_status_cell::SERVICE_DOWN;
  m_nDefenseFlags = 0;
  m_nAttackFlags = 0;
  m_nAttackPoints = 0;
  m_nDefensePoints = 0;
  m_flags_stollen.store(0);
}

const std::string &service_status_cell::serviceId() {
  return m_sServiceId;
}

void service_status_cell::setDefenseFlags(int nDefenseFlags) {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_nDefenseFlags = nDefenseFlags;
}

int service_status_cell::getDefenseFlags() {
  return m_nDefenseFlags;
}

void service_status_cell::incrementDefenseFlags() {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_nDefenseFlags++;
}

void service_status_cell::setDefensePoints(int nDefensePoints) {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_nDefensePoints = nDefensePoints;
}

int service_status_cell::getDefensePoints() {
  return m_nDefensePoints;
}

void service_status_cell::addDefensePoints(int nDefensePoints) {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_nDefensePoints += nDefensePoints;
}

void service_status_cell::setAttackFlags(int nAttackFlags) {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_nAttackFlags = nAttackFlags;
}

int service_status_cell::getAttackFlags() {
  return m_nAttackFlags;
}

void service_status_cell::incrementAttackFlags() {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_nAttackFlags++;
}

void service_status_cell::setFlagsStollen(int val) {
  m_flags_stollen.store(val);
}

int service_status_cell::getFlagsStollen() {
  return m_flags_stollen.load();
}

void service_status_cell::decrementFlagsStollen() {
  m_flags_stollen.store(m_flags_stollen.load() - 1);
}

void service_status_cell::setAttackPoints(int nAttackPoints) {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_nAttackPoints = nAttackPoints;
}

int service_status_cell::getAttackPoints() {
  return m_nAttackPoints;
}

void service_status_cell::addAttackPoints(int nAttackPoints) {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_nAttackPoints += nAttackPoints;
}

void service_status_cell::setFlagsPutAllResultsCounter(int nFlagsPutAllResultsCounter) {
  m_nFlagsPutAllResultsCounter = nFlagsPutAllResultsCounter;
}

void service_status_cell::setFlagsPutSuccessResultsCounter(int nFlagsPutSuccessResultsCounter) {
  m_nFlagsPutSuccessResultsCounter = nFlagsPutSuccessResultsCounter;
}

void service_status_cell::incrementPutFlagSuccess() {
  m_nFlagsPutSuccessResultsCounter++;
  m_nFlagsPutAllResultsCounter++;
}

void service_status_cell::incrementPutFlagFail() {
  m_nFlagsPutAllResultsCounter++;
}

int service_status_cell::calculateSLA() {
  if (m_nFlagsPutAllResultsCounter == 0) {
    // if (m_nFlagsPutSuccessResultsCounter != 0) {
    //   ctf01d::log::warn(TAG, "Could not possible situation!");
    // }
    return 100;
  }
  return (m_nFlagsPutSuccessResultsCounter*100) / m_nFlagsPutAllResultsCounter;
}

void service_status_cell::setStatus(const std::string &sStatus) {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_sStatus = sStatus;
  if (sStatus != service_status_cell::SERVICE_UP) {
    m_nUpPointTimeInSec = WsjcppCore::getCurrentTimeInSeconds();
  }
}

std::string service_status_cell::status() {
  return m_sStatus;
}

} // namespace ctf01d