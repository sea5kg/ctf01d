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

#include "ctf01d_game_config.h"
#include "ctf01d/include/ctf01d_globals.h"
#include <sea5kg_logger.h>
#include <wsjcpp_core.h>
#include <vector>
#include <algorithm>

namespace ctf01d {

game_config::game_config() {
  m_id = ctf01d::var_string::create({ctf01d::yaml_keys::ID}, "test", m_vars);
  m_name = ctf01d::var_string::create({ctf01d::yaml_keys::NAME}, "Test", m_vars);

  m_start_utc = ctf01d::var_datetime::create({ctf01d::yaml_keys::START_UTC}, "2023-11-12 16:00:00", m_vars);
  m_end_utc = ctf01d::var_datetime::create({ctf01d::yaml_keys::END_UTC}, "2030-11-12 22:00:00", m_vars);
  m_coffee_break_start_utc = ctf01d::var_datetime::create({ctf01d::yaml_keys::COFFEE_BREAK, ctf01d::yaml_keys::START_UTC}, "2023-11-12 20:00:00", m_vars);
  m_coffee_break_end_utc = ctf01d::var_datetime::create({ctf01d::yaml_keys::COFFEE_BREAK, ctf01d::yaml_keys::END_UTC}, "2023-11-12 21:00:00", m_vars);

  m_flag_lifetime_in_seconds = ctf01d::var_int::create({ctf01d::yaml_keys::FLAG_LIFETIME_IN_SECONDS}, DEFAULT_FLAG_LIFETIME_IN_SECONDS, m_vars);
  m_flag_lifetime_in_seconds->set_minimum(1);
  m_flag_lifetime_in_seconds->set_maximum(ctf01d::MAX_FLAG_LIFETIME_SECONDS);

  m_flag_cost_in_points = ctf01d::var_int::create({ctf01d::yaml_keys::FLAG_COST_IN_POINTS}, 100, m_vars);
  m_flag_cost_in_points->set_minimum(1);
  m_flag_cost_in_points->set_maximum(ctf01d::MAX_FLAG_COST_IN_POINTS);

  m_has_coffee_break = false;

  nlohmann::json options;
  options["size"] = 10;
  m_default_flag_id_generator = ctf01d::flag_id_generators::random_string(options);
}

game_config::~game_config() {
  m_vars.clear();
}

bool game_config::read(WsjcppYamlCursor cursor, const std::string &work_dir, std::string &err) {
  m_work_dir = work_dir;

  if (!cursor.hasKey(ctf01d::yaml_keys::GAME)) {
    err = "Missing root key '" + ctf01d::yaml_keys::GAME + "'";
    return false;
  }
  cursor = cursor[ctf01d::yaml_keys::GAME];

  // check type
  static const std::vector<std::string> allowed_keys = {
    ctf01d::yaml_keys::ID,
    ctf01d::yaml_keys::NAME,
    ctf01d::yaml_keys::START_UTC,
    ctf01d::yaml_keys::END_UTC,
    ctf01d::yaml_keys::COFFEE_BREAK,
    ctf01d::yaml_keys::FLAG_LIFETIME_IN_SECONDS,
    ctf01d::yaml_keys::FLAG_COST_IN_POINTS,
  };

  if (!m_vars.read(cursor, err)) {
    return false;
  }

  sea5kg::log::info(TAG, "Game start: " + start_utc());
  sea5kg::log::info(TAG, "Game start (UNIX timestamp): " + std::to_string(start_utc_in_seconds()));
  sea5kg::log::info(TAG, "Game end: " + end_utc());
  sea5kg::log::info(TAG, "Game end (UNIX timestamp): " + std::to_string(end_utc_in_seconds()));

  if (end_utc_in_seconds() <= start_utc_in_seconds()) {
    err = "game.end must be gather then game.start";
    return false;
  }

  sea5kg::log::info(TAG, "game.coffee_break_start: " + coffee_break_start_utc());
  sea5kg::log::info(TAG, "Game coffee break start (UNIX timestamp): " + std::to_string(coffee_break_start_utc_in_seconds()));

  sea5kg::log::info(TAG, "game.coffee_break_end: " + coffee_break_end_utc());
  sea5kg::log::info(TAG, "Game coffee break end (UNIX timestamp): " + std::to_string(coffee_break_end_utc_in_seconds()));

  if (start_utc_in_seconds() < coffee_break_start_utc_in_seconds()
    && coffee_break_start_utc_in_seconds() < end_utc_in_seconds()
    && start_utc_in_seconds() < coffee_break_end_utc_in_seconds()
    && coffee_break_end_utc_in_seconds() < end_utc_in_seconds()
  ) {
    sea5kg::log::success(TAG, "Oh! Game has coffee break! nice!");
    m_has_coffee_break = true;
  }

  return true;
}

std::string game_config::id() const {
  return m_id->value();
}

std::string game_config::name() const {
  return m_name->value();
}

std::string game_config::start_utc() const {
  return m_start_utc->value();
}

int game_config::start_utc_in_seconds() const {
  return m_start_utc->value_in_seconds();
}

std::string game_config::end_utc() const {
  return m_end_utc->value();
}

int game_config::end_utc_in_seconds() const {
  return m_end_utc->value_in_seconds();
}

std::string game_config::coffee_break_start_utc() const {
  return m_coffee_break_start_utc->value();
}

int game_config::coffee_break_start_utc_in_seconds() const {
  return m_coffee_break_start_utc->value_in_seconds();
}

std::string game_config::coffee_break_end_utc() const {
  return m_coffee_break_end_utc->value();
}

int game_config::coffee_break_end_utc_in_seconds() const {
  return m_coffee_break_end_utc->value_in_seconds();
}

bool game_config::has_coffee_break() const {
  return m_has_coffee_break;
}

int game_config::flag_lifetime_in_seconds() const {
  return m_flag_lifetime_in_seconds->value();
}

std::shared_ptr<ctf01d::var_int> game_config::flag_cost_in_points() const {
  return m_flag_cost_in_points;
}

std::shared_ptr<ctf01d::flag_id_generator> game_config::default_flag_id_generator() {
  return m_default_flag_id_generator;
}

} // namespace ctf01d
