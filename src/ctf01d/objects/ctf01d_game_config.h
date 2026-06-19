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
#include <memory>
#include <wsjcpp_yaml.h>
#include "ctf01d_var.h"
#include "ctf01d/utils/ctf01d_flag_id_generators.h"

namespace ctf01d {

class game_config {
public:
  game_config();
  ~game_config();
  bool read(WsjcppYamlCursor cursor, const std::string &work_dir, std::string &err);

  std::string id() const;
  std::string name() const;
  std::string start_utc() const;
  int start_utc_in_seconds() const;
  std::string end_utc() const;
  int end_utc_in_seconds() const;
  std::string coffee_break_start_utc() const;
  int coffee_break_start_utc_in_seconds() const;
  std::string coffee_break_end_utc() const;
  int coffee_break_end_utc_in_seconds() const;
  bool has_coffee_break() const;
  int flag_lifetime_in_seconds() const;
  std::shared_ptr<ctf01d::var_int> flag_cost_in_points() const;
  std::shared_ptr<ctf01d::flag_id_generator> default_flag_id_generator();

private:
  std::string TAG;
  std::string m_work_dir;
  ctf01d::scope_vars m_vars = ctf01d::scope_vars("game_config");
  std::shared_ptr<ctf01d::var_string> m_id;
  std::shared_ptr<ctf01d::var_string> m_name;
  std::shared_ptr<ctf01d::var_int> m_flag_lifetime_in_seconds;
  std::shared_ptr<ctf01d::var_int> m_flag_cost_in_points;
  std::shared_ptr<ctf01d::var_datetime> m_start_utc;
  std::shared_ptr<ctf01d::var_datetime> m_end_utc;
  bool m_has_coffee_break;
  std::shared_ptr<ctf01d::var_datetime> m_coffee_break_start_utc;
  std::shared_ptr<ctf01d::var_datetime> m_coffee_break_end_utc;
  std::shared_ptr<ctf01d::flag_id_generator> m_default_flag_id_generator;
};

} // namespace ctf01d
