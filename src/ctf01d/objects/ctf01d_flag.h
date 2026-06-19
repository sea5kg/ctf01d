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

namespace ctf01d {

class flag {
public:
  flag();
  void generate_random_flag(
    int flag_lifetime_in_seconds,
    const std::string &team_id, const std::string &service_id, int game_start_utc_in_seconds);

  void generate_id();
  void set_id(const std::string &id);
  const std::string &id() const;

  void generate_value(int game_start_utc_in_seconds);
  void set_value(const std::string &sValue);
  const std::string &value() const;

  void set_team_id(const std::string &team_id);
  const std::string &team_id() const;

  void set_service_id(const std::string &service_id);
  const std::string &service_id() const;

  void set_time_start_in_milliseconds(long time_start);
  long time_start_in_milliseconds() const;

  void set_time_end_in_milliseconds(long time_end);
  long time_end_in_milliseconds() const;

  void copy_from(const ctf01d::flag &flag);

private:
  std::string m_id;
  std::string m_value;
  std::string m_team_id;
  std::string m_service_id;
  long m_time_start_in_milliseconds;
  long m_time_end_in_milliseconds;
};

} // namespace ctf01d
