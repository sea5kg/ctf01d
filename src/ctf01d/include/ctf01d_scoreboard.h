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

#include <optional>
#include <string>
#include <json.hpp>
#include "ctf01d/objects/ctf01d_flag.h"

namespace ctf01d {

class scoreboard {
public:
  static std::string name() { return "scoreboard"; }
  virtual void set_service_status(const std::string &team_id, const std::string &service_id, const std::string &status) = 0;
  virtual void insert_flag_attempt(const std::string &thief_team_id, const std::string &flag_value, const std::string &request_ip) = 0;
  virtual void init_state_from_storage() = 0;
  // Returns flag points on success; std::nullopt if this team has already
  // stolen the flag (dedup check happens under the same lock as the insert
  // so concurrent submissions can't double-credit).
  virtual std::optional<int> increment_attack_score(const ctf01d::flag &flag, const std::string &team_id) = 0;
  virtual void increment_defense_score(const ctf01d::flag &flag) = 0;
  virtual void increment_flags_putted_and_service_up(const ctf01d::flag &flag) = 0;
  virtual void insert_flag_put_fail(const ctf01d::flag &flag, const std::string &service_status, const std::string &description_status) = 0;
  virtual std::string service_status(const std::string &team_id, const std::string &service_id) = 0;
  virtual const nlohmann::json &to_json() = 0;
};

} // namespace ctf01d
