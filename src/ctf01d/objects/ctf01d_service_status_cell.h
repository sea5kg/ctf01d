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

#include <string>
#include <mutex>

enum class Ctf01dServiceStatus : char {
  SERVICE_UP = 'u',
  SERVICE_DOWN = 'd',
  SERVICE_MUMBLE = 'm',
  SERVICE_CORRUPT = 'c',
  SERVICE_SHIT = 's',
  SERVICE_WAIT = 'w',
  SERVICE_COFFEE_BREAK = 'b'
};

class Ctf01dServiceStatusCell {
public:
  // enum for service status
  static const std::string SERVICE_UP;
  static const std::string SERVICE_DOWN;
  static const std::string SERVICE_MUMBLE;
  static const std::string SERVICE_CORRUPT;
  static const std::string SERVICE_SHIT;
  static const std::string SERVICE_WAIT;
  static const std::string SERVICE_COFFEE_BREAK;

  Ctf01dServiceStatusCell(const std::string &service_id);
  const std::string &serviceId();

  void setDefenseFlags(int nDefenseFlags);
  int getDefenseFlags();
  void incrementDefenseFlags();

  void setDefensePoints(int nDefensePoints);
  int getDefensePoints();
  void addDefensePoints(int nDefensePoints);

  void setAttackFlags(int nAttackFlags);
  int getAttackFlags();
  void incrementAttackFlags();

  void setAttackPoints(int nAttackPoints);
  int getAttackPoints();
  void addAttackPoints(int nAttackPoints);

  void setFlagsPutAllResultsCounter(int nFlagsPutAllResultsCounter);
  void setFlagsPutSuccessResultsCounter(int nFlagsPutSuccessResultsCounter);
  void incrementPutFlagSuccess();
  void incrementPutFlagFail();
  int calculateSLA();

  void setStatus(const std::string &sStatus);
  std::string status();

private:
  std::string TAG;
  std::mutex m_mutexServiceStatus;
  std::string m_sServiceId;
  std::string m_sStatus; // may be char[10] ?
  int m_nDefenseFlags;
  int m_nAttackFlags;
  int m_nAttackPoints;
  int m_nDefensePoints;
  int m_nUpPointTimeInSec;

  // for SLA / uptime
  int m_nFlagsPutAllResultsCounter;
  int m_nFlagsPutSuccessResultsCounter;
};