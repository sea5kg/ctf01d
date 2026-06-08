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
#include <json.hpp>

class Ctf01dServiceStatistics {
public:
  Ctf01dServiceStatistics(const std::string &sServiceId);
  int getAllStolenFlagsForService();
  void doIncrementStolenFlagsForService();
  void setStolenFlagsForService(int nStolenFlags);

  int getAllDefenseFlagsForService();
  void doIncrementDefenseFlagsForService();

  void setDefenseFlagsForService(int nAllDefenseFlagsForService);

  std::string getFirstBloodTeamId();
  long getFirstBloodTime();
  void setFirstBloodTeamId(const std::string &sFirstBlood, long nDateACtion);
  void updateJsonServiceStatistics(nlohmann::json &jsonCosts);

private:
  std::string TAG;
  std::string m_sServiceId;
  std::string m_sFirstBloodTeamId;
  long m_nFirstBloodTimeInSeconds;

  int m_nAllStolenFlagsForService;
  int m_nAllDefenseFlagsForService;
};