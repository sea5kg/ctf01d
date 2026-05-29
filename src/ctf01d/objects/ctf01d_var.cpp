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

#include "ctf01d_var.h"
#include <wsjcpp_core.h>
#include <cstring>

namespace ctf01d {

// ---------------------------------------------------------------------
// ctf01d::var

var::var(const std::vector<std::string> &path_name, ctf01d::var_type t)
: m_path_name(path_name), m_t(t) {
  m_name = "";
  for (int i = 0; i < m_path_name.size(); ++i) {
    if (i != 0) {
      m_name += ".";
    }
    m_name += m_path_name[i];
  }
}

std::string var::name() const {
  return m_name;
}

ctf01d::var_type var::type() const {
  return m_t;
}

bool var::read(WsjcppYaml &yaml, std::string &err) {
  // will be overriden by type
  return false;
}

std::string var::to_string() {
  return "unknown";
}

WsjcppYamlCursor var::cursorByPath(WsjcppYaml &yaml, std::string &err) {
  auto cur = yaml.getCursor();
  for (int i = 0; i < m_path_name.size(); ++i) {
    std::string key = m_path_name[i];
    if (!cur.hasKey(key)) {
      return cur[key];
    }
    cur = cur[key];
  }
  return cur;
}

// ---------------------------------------------------------------------
// ctf01d::var_int

var_int::var_int(const std::vector<std::string> &path_name, int default_value)
: ctf01d::var(path_name, ctf01d::var_type::INTEGER)
  ,m_value_init(false)
  ,m_default(default_value)
  ,m_check_minimum(false)
  ,m_check_maximum(false)
{

}

// static
std::shared_ptr<var_int> var_int::create(const std::vector<std::string> &path_name, int default_value) {
  return std::make_shared<var_int>(path_name, default_value);
}

bool var_int::read(WsjcppYaml &yaml, std::string &err) {
  auto cursor = cursorByPath(yaml, err);
  if (!cursor.isValue()) {
    err = "Not found '" + name() + "'";
    return false;
  }
  return set_value(cursor.valInt(), err);
}

std::string var_int::to_string() {
  return std::to_string(value());
}

int var_int::default_value() const {
  return m_default;
}

int var_int::value() const {
  return m_value_init ? m_value : m_default;
}

bool var_int::set_value(int val, std::string &err) {
  if (m_check_minimum && val < m_minimum) {
    err = "Value '" + name() + "' must be equal or more than " + std::to_string(m_minimum);
    WsjcppLog::err("var_int", err);
    return false;
  }
  if (m_check_maximum && val > m_maximum) {
    err = "Value '" + name() + "' must be less or equal than " + std::to_string(m_maximum);
    WsjcppLog::err("var_int", err);
    return false;
  }
  m_value = val;
  m_value_init = true;
  return true;
}

void var_int::set_minimum(int val) {
  m_minimum = val;
  m_check_minimum = true;
}

void var_int::set_maximum(int val) {
  m_maximum = val;
  m_check_maximum = true;
}

// ---------------------------------------------------------------------
// ctf01d::var_string

var_string::var_string(const std::vector<std::string> &path_name, const std::string &default_value)
: ctf01d::var(path_name, ctf01d::var_type::STRING), m_value_init(false), m_default_value(default_value) {
}

// static
std::shared_ptr<var_string> var_string::create(const std::vector<std::string> &path_name, const std::string &default_value) {
  return std::make_shared<var_string>(path_name, default_value);
}

bool var_string::read(WsjcppYaml &yaml, std::string &err) {
  auto cur = cursorByPath(yaml, err);
  if (cur.isValue()) {
    setValue(cur.valStr());
    return true;
  }
  return false;
}

std::string var_string::to_string() {
  return "'" +  value() + "'";
}

std::string var_string::defaultValue() const {
  return m_default_value;
}

std::string var_string::value() const {
  return m_value_init ? m_value : m_default_value;
}

void var_string::setValue(const std::string &val) {
  m_value = val;
  m_value_init = true;
}

// ---------------------------------------------------------------------
// ctf01d::var_bool

var_bool::var_bool(const std::vector<std::string> &path_name, bool default_value)
: ctf01d::var(path_name, ctf01d::var_type::BOOLEAN), m_default(default_value), m_value_init(false) {

}

// static
std::shared_ptr<var_bool> var_bool::create(const std::vector<std::string> &path_name, bool default_value) {
  return std::make_shared<var_bool>(path_name, default_value);
}

bool var_bool::read(WsjcppYaml &yaml, std::string &err) {
  auto cursor = cursorByPath(yaml, err);
  if (cursor.isValue()) {
    return set_value(cursor.valBool(), err);
  }
  return true;
}

std::string var_bool::to_string() {
  return value() ? "yes" : "no";
}

bool var_bool::default_value() const {
  return m_default;
}

bool var_bool::value() const {
  return m_value_init ? m_value : m_default;
}

bool var_bool::set_value(bool val, std::string &err) {
  m_value = val;
  m_value_init = true;
  return true;
}

// ---------------------------------------------------------------------
// ctf01d::var_dir

var_dir::var_dir(const std::vector<std::string> &path_name, const std::string &default_value, const std::string &root_dir)
: ctf01d::var(path_name, ctf01d::var_type::STRING), m_value_init(false) {
  m_root_dir = root_dir;
  m_default = to_absolute_path(default_value);
}

// static
std::shared_ptr<var_dir> var_dir::create(const std::vector<std::string> &path_name, const std::string &default_value, const std::string &root_dir) {
  return std::make_shared<var_dir>(path_name, default_value, root_dir);
}

bool var_dir::read(WsjcppYaml &yaml, std::string &err) {
  auto cur = cursorByPath(yaml, err);
  if (cur.isValue()) {
    return set_value(cur.valStr(), err);
  }
  return false;
}

std::string var_dir::to_string() {
  return "'" +  value() + "'";
}

void var_dir::set_root_dir(const std::string &val) {
  m_root_dir = val;
  m_value = to_absolute_path(m_value);
  m_default = to_absolute_path(m_value);
}

std::string var_dir::default_value() const {
  return m_default;
}

std::string var_dir::value() const {
  return m_value_init ? m_value : m_default;
}

bool var_dir::set_value(const std::string &val, std::string &err) {
  std::string new_val = to_absolute_path(val);

  if (!WsjcppCore::dirExists(new_val)) {
    err = "Directory '" + new_val + "' does not exists";
    WsjcppLog::err("var_dir", err);
    return false;
  }
  m_value = new_val;
  m_value_init = true;
  return true;
}

std::string var_dir::to_absolute_path(const std::string &val) {
  std::string ret = val;
  if (ret.size() > 0 && ret[0] != '/') {
    ret = m_root_dir + "/" + ret;
  }
  return wsjcpp::normalizeFilePath(ret);
}

} // namespace ctf01d
