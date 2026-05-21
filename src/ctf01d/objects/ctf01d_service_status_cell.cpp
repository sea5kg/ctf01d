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

const std::string Ctf01dServiceStatusCell::SERVICE_UP = "up";
const std::string Ctf01dServiceStatusCell::SERVICE_DOWN = "down";
const std::string Ctf01dServiceStatusCell::SERVICE_MUMBLE = "mumble";
const std::string Ctf01dServiceStatusCell::SERVICE_CORRUPT = "corrupt";
const std::string Ctf01dServiceStatusCell::SERVICE_SHIT = "shit";
const std::string Ctf01dServiceStatusCell::SERVICE_WAIT = "wait";
const std::string Ctf01dServiceStatusCell::SERVICE_COFFEE_BREAK = "coffeebreak";

Ctf01dServiceStatusCell::Ctf01dServiceStatusCell(const std::string &sServiceId) {
  m_nUpPointTimeInSec = WsjcppCore::getCurrentTimeInSeconds();
  TAG = "Ctf01dServiceStatusCell-" + sServiceId;
  m_sServiceId = sServiceId;
  m_sStatus = Ctf01dServiceStatusCell::SERVICE_DOWN;
  m_nDefenseFlags = 0;
  m_nAttackFlags = 0;
  m_nAttackPoints = 0;
  m_nDefensePoints = 0;
}

const std::string &Ctf01dServiceStatusCell::serviceId() {
  return m_sServiceId;
}

void Ctf01dServiceStatusCell::setDefenseFlags(int nDefenseFlags) {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_nDefenseFlags = nDefenseFlags;
}

int Ctf01dServiceStatusCell::getDefenseFlags() {
  return m_nDefenseFlags;
}

void Ctf01dServiceStatusCell::incrementDefenseFlags() {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_nDefenseFlags++;
}

void Ctf01dServiceStatusCell::setDefensePoints(int nDefensePoints) {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_nDefensePoints = nDefensePoints;
}

int Ctf01dServiceStatusCell::getDefensePoints() {
  return m_nDefensePoints;
}

void Ctf01dServiceStatusCell::addDefensePoints(int nDefensePoints) {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_nDefensePoints += nDefensePoints;
}

void Ctf01dServiceStatusCell::setAttackFlags(int nAttackFlags) {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_nAttackFlags = nAttackFlags;
}

int Ctf01dServiceStatusCell::getAttackFlags() {
  return m_nAttackFlags;
}

void Ctf01dServiceStatusCell::incrementAttackFlags() {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_nAttackFlags++;
}

void Ctf01dServiceStatusCell::setAttackPoints(int nAttackPoints) {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_nAttackPoints = nAttackPoints;
}

int Ctf01dServiceStatusCell::getAttackPoints() {
  return m_nAttackPoints;
}

void Ctf01dServiceStatusCell::addAttackPoints(int nAttackPoints) {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_nAttackPoints += nAttackPoints;
}

void Ctf01dServiceStatusCell::setFlagsPutAllResultsCounter(int nFlagsPutAllResultsCounter) {
  m_nFlagsPutAllResultsCounter = nFlagsPutAllResultsCounter;
}

void Ctf01dServiceStatusCell::setFlagsPutSuccessResultsCounter(int nFlagsPutSuccessResultsCounter) {
  m_nFlagsPutSuccessResultsCounter = nFlagsPutSuccessResultsCounter;
}

void Ctf01dServiceStatusCell::incrementPutFlagSuccess() {
  m_nFlagsPutSuccessResultsCounter++;
  m_nFlagsPutAllResultsCounter++;
}

void Ctf01dServiceStatusCell::incrementPutFlagFail() {
  m_nFlagsPutAllResultsCounter++;
}

int Ctf01dServiceStatusCell::calculateSLA() {
  if (m_nFlagsPutAllResultsCounter == 0) {
    // if (m_nFlagsPutSuccessResultsCounter != 0) {
    //   WsjcppLog::warn(TAG, "Could not possible situation!");
    // }
    return 100;
  }
  return (m_nFlagsPutSuccessResultsCounter*100) / m_nFlagsPutAllResultsCounter;
}

void Ctf01dServiceStatusCell::setStatus(const std::string &sStatus) {
  std::lock_guard<std::mutex> lock(m_mutexServiceStatus);
  m_sStatus = sStatus;
  if (sStatus != Ctf01dServiceStatusCell::SERVICE_UP) {
    m_nUpPointTimeInSec = WsjcppCore::getCurrentTimeInSeconds();
  }
}

std::string Ctf01dServiceStatusCell::status() {
  return m_sStatus;
}
