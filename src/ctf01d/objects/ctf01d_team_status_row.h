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
#include <map>
#include "ctf01d_service_status_cell.h"

class Ctf01dTeamStatusRow {
public:
  Ctf01dTeamStatusRow(const std::string &sTeamId, int nGameStartInSec, int nGameEndInSec);
  const std::string &teamId();

  void setPlace(int nPlace);
  int getPlace();

  void setPoints(int nPoints);
  int getPoints();

  void setServiceStatus(const std::string &service_id, std::string sStatus);
  std::string serviceStatus(const std::string &service_id);

  void setTries(int nScore);
  int tries();

  std::string servicesToString();

  void incrementDefense(const std::string &service_id, int nFlagPoints);
  int getDefenseFlags(const std::string &service_id);
  int getDefensePoints(const std::string &service_id);
  void setServiceDefenseFlagsAndPoints(const std::string &service_id, int nDefenseFlags, int nDefensePoints);

  void decrementFlagStollen(const std::string &service_id);
  int getFlagsStollen(const std::string &service_id);
  void setFlagsStollen(const std::string &service_id, int val);

  void incrementAttack(const std::string &service_id, int nFlagPoints);
  void setServiceAttackFlagsAndPoints(const std::string &service_id, int nAttackFlags, int nAttackPoints);
  int getAttackFlags(const std::string &service_id);
  int getAttackPoints(const std::string &service_id);

  void setServiceFlagsForCalculateSLA(const std::string &service_id, int nPutsFlagsAllResults, int nPutsFlagsSuccessResults);
  void incrementPutFlagSuccess(const std::string &service_id);
  void incrementPutFlagFail(const std::string &service_id);
  int calculateSLA(const std::string &service_id);

  void updatePoints();

private:
  std::mutex m_mutex;
  std::string TAG;
  std::string m_sTeamId;
  std::atomic<int> m_place;
  int m_nPoints;
  int m_nTries;
  std::map<std::string, Ctf01dServiceStatusCell *> m_mapServicesStatus;
};
