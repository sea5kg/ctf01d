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

#include <vector>
#include <json.hpp>
#include "ctf01d/objects/ctf01d_flag.h"

namespace ctf01d {

class database {
public:
  static std::string name() { return "database"; }

  virtual void insert_to_flags_checker_put_result(ctf01d::flag flag, std::string result) = 0;
  virtual int number_of_flags_checker_put_all_results(std::string team_id, std::string service_id) = 0;
  virtual int number_of_flags_checker_put_success_result(std::string team_id, std::string service_id) = 0;
  virtual void insertToFlagsDefense(ctf01d::flag flag, int nPoints) = 0;
  virtual int number_of_flags_defense(std::string team_id, std::string service_id) = 0;
  virtual int sum_points_of_flags_defense(std::string team_id, std::string service_id) = 0;
  virtual int number_of_defense_flag_for_service(std::string service_id) = 0;
  virtual void insert_flag_check_fail(ctf01d::flag flag, std::string sReason) = 0;
  virtual int number_of_flags_stollen(std::string team_id, std::string service_id) = 0;
  virtual int number_of_flags_stollen_by_victim(std::string team_id, std::string service_id) = 0;
  virtual int sum_points_of_flags_stolen(std::string team_id, std::string service_id) = 0;
  virtual int number_of_stolen_flags_for_service(std::string service_id) = 0;
  virtual std::pair<std::string, long> get_first_blood_from_stolen_flags_for_service(std::string service_id) = 0;
  virtual void insert_to_flags_stolen(ctf01d::flag flag, std::string team_id, int nPoints, long date_action, int victim_place_in_scoreboard, int thief_place_in_scoreboard) = 0;
  virtual bool is_already_stole(ctf01d::flag flag, std::string team_id) = 0;
  virtual bool is_somebody_stole(ctf01d::flag flag) = 0;
};

} // namespace ctf01d
