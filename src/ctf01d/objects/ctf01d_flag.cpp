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
#include <sea5kg_logger.h>
#include <wsjcpp_core.h>
#include <cstring>

namespace ctf01d {

flag::flag() {
  // flag id
  m_id = "qweRT12345";
  // flag format
  m_value = "c01d0000-0000-0000-0000-000000000000";
  m_team_id = "";
  m_service_id = "";
  m_time_start_in_milliseconds = 0;
  m_time_end_in_milliseconds = 0;
}

void flag::generate_random_flag(
  std::shared_ptr<ctf01d::flag_id_generator> flag_id_generator,
  int flag_lifetime_in_seconds,
  const std::string &team_id,
  const std::string &service_id,
  int game_start_utc_in_seconds
) {
  long time_start_in_milliseconds = WsjcppCore::getCurrentTimeInMilliseconds();
  long time_end_in_milliseconds = time_start_in_milliseconds + long(flag_lifetime_in_seconds)*1000;
  set_time_start_in_milliseconds(time_start_in_milliseconds);
  set_time_end_in_milliseconds(time_end_in_milliseconds);

  m_id = flag_id_generator->generate();
  generate_value(game_start_utc_in_seconds);
  m_team_id = team_id;
  m_service_id = service_id;
}

void ctf01d::flag::set_id(const std::string &id) {
  m_id = id;
}

const std::string &ctf01d::flag::id() const {
  return m_id;
}

void ctf01d::flag::generate_value(int game_start_utc_in_seconds) {
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
      m_value[i] = sAlphabet[rand() % sAlphabet.length()];
    }
  }

  // set timepoint
  int dt = m_time_start_in_milliseconds / 1000 - game_start_utc_in_seconds;
  std::string sTimePoint = std::to_string(dt);
  int nTimePointLen = sTimePoint.size();
  if (nTimePointLen > 8) {
    sea5kg::log::critical("ctf01d::flag::generate_value", "Really game was started more then 3 years ago ??? got value: " + sTimePoint);
  }
  int nPos = m_value.size() - 1;
  for (int i = nTimePointLen - 1; i >= 0; i--) {
    m_value[nPos] = sTimePoint[i];
    nPos--;
  }
  // 03268167

  // std::cout << "sTimePoint: " << sTimePoint << "\n";
  // this->setValue(std::string(sUuid) + sTimePoint);
}

void ctf01d::flag::set_value(const std::string &val) {
  // TODO validate format
  // c01d...00000000 - prefix and time
  m_value = val;
}

const std::string &ctf01d::flag::value() const {
  return m_value;
}

void ctf01d::flag::set_team_id(const std::string &team_id) {
  m_team_id = team_id;
}

const std::string &ctf01d::flag::team_id() const {
  return m_team_id;
}

void ctf01d::flag::set_service_id(const std::string &service_id) {
  m_service_id = service_id;
}

const std::string &ctf01d::flag::service_id() const {
  return m_service_id;
}

void ctf01d::flag::set_time_start_in_milliseconds(long nTimeStartInMs) {
  m_time_start_in_milliseconds = nTimeStartInMs;
}

long ctf01d::flag::time_start_in_milliseconds() const {
  return m_time_start_in_milliseconds;
}

void ctf01d::flag::set_time_end_in_milliseconds(long time_end) {
  m_time_end_in_milliseconds = time_end;
}

long ctf01d::flag::time_end_in_milliseconds() const {
  return m_time_end_in_milliseconds;
}

void ctf01d::flag::copy_from(const ctf01d::flag &flag) {
  this->set_id(flag.id());
  this->set_value(flag.value());
  this->set_service_id(flag.service_id());
  this->set_team_id(flag.team_id());
  this->set_time_start_in_milliseconds(flag.time_start_in_milliseconds());
  this->set_time_end_in_milliseconds(flag.time_end_in_milliseconds());
}

} // namespace ctf01d
