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

#include "ctf01d/objects/ctf01d_team_config.h"
#include "ctf01d/objects/ctf01d_service_config.h"
#include "ctf01d/objects/ctf01d_var.h"
#include "ctf01d/utils/ctf01d_flag_id_generators.h"

namespace ctf01d {

class listener_config_changed {
public:
  virtual void config_changed() = 0;
};

class config {
public:
  static std::string name() { return "config"; }

  virtual void set_ctf01d_version(const std::string &ctf01d_version) = 0;
  virtual std::string ctf01d_version() = 0;
  virtual void set_work_dir(const std::string &sWorkDir) = 0;
  virtual std::string get_work_dir() = 0;
  virtual bool apply_config() = 0;
  virtual const std::vector<ctf01d::service_config> &services() = 0;
  virtual const std::vector<ctf01d::team_config> &teams() = 0;
  virtual bool contains_team_id(const std::string &team_id) const = 0;
  virtual std::string find_team_id_by_subnet(const std::string &ip) const = 0;
  virtual int scoreboard_port() const = 0;
  virtual std::string scoreboard_html_folder() const = 0;
  virtual bool scoreboard_auto_detection_team_id_by_subnet_ip() const = 0;
  virtual bool scoreboard_random() const = 0;
  virtual bool scoreboard_metrics_enabled() const = 0;
  virtual std::string scoreboard_metrics_allowed_for() const = 0;
  virtual std::string game_id() const = 0;
  virtual std::string game_name() const = 0;
  virtual int flag_lifetime_in_seconds() const = 0;
  virtual std::shared_ptr<ctf01d::var_int> get_flag_cost_in_points() const = 0;
  virtual int game_start_utc_in_seconds() const = 0;
  virtual int game_end_utc_in_seconds() const = 0;
  virtual bool game_has_coffee_break() const = 0;
  virtual int game_coffee_break_start_utc_in_seconds() const = 0;
  virtual int game_coffee_break_end_utc_in_seconds() const = 0;
  virtual std::shared_ptr<ctf01d::flag_id_generator> default_flag_id_generator() = 0;
  virtual void add_listener(ctf01d::listener_config_changed *listener) = 0;
};

} // namespace ctf01d
