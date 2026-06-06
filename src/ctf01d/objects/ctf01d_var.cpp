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
#include <date.h> // HowardHinnant_date
#include <regex>

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

bool var::read(WsjcppYamlCursor &cursor, std::string &err) {
  // will be overriden by type
  return false;
}

std::string var::to_string() {
  return "unknown";
}

WsjcppYamlCursor var::cursor_by_path(WsjcppYamlCursor &cursor, std::string &err) {
  auto _cursor = cursor;
  for (int i = 0; i < m_path_name.size(); ++i) {
    std::string key = m_path_name[i];
    if (!_cursor.hasKey(key)) {
      return _cursor[key];
    }
    _cursor = _cursor[key];
  }
  return _cursor;
}

// ---------------------------------------------------------------------
// ctf01d::scope_vars

scope_vars::scope_vars(const std::string &scope_name) : m_scope_name(scope_name) {}

void scope_vars::add_var(std::shared_ptr<ctf01d::var> v) {
  m_vars.push_back(v);
}

void scope_vars::clear() {
  m_vars.clear();
}

bool scope_vars::read(WsjcppYamlCursor &cursor, std::string &err) {
  bool var_errors = false;
  for (int i = 0; i < m_vars.size(); ++i) {
    std::shared_ptr<ctf01d::var> var = m_vars[i];
    std::string err;
    if (!var->read(cursor, err)) {
      var_errors = true;
      WsjcppLog::err(m_scope_name, err);
      continue;
    }
    WsjcppLog::info(m_scope_name, var->name() + ": " + var->to_string());
  }
  if (var_errors) {
    return false;
  }
  return true;
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
std::shared_ptr<var_int> var_int::create(
  const std::vector<std::string> &path_name,
  int default_value,
  ctf01d::scope_vars &sc_vars
) {
  auto ret = std::make_shared<var_int>(path_name, default_value);
  sc_vars.add_var(ret);
  return ret;
}

bool var_int::read(WsjcppYamlCursor &cursor, std::string &err) {
  auto _cursor = cursor_by_path(cursor, err);
  if (!_cursor.isValue()) {
    err = "Not found '" + name() + "'";
    return false;
  }
  return set_value(_cursor.valInt(), err);
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
std::shared_ptr<var_string> var_string::create(
  const std::vector<std::string> &path_name,
  const std::string &default_value,
  ctf01d::scope_vars &sc_vars
) {
  auto ret = std::make_shared<var_string>(path_name, default_value);
  sc_vars.add_var(ret);
  return ret;
}

bool var_string::read(WsjcppYamlCursor &cursor, std::string &err) {
  auto cur = cursor_by_path(cursor, err);
  if (cur.isValue()) {
    set_value(cur.valStr());
    return true;
  }
  err = "Not value for '" + name() + "'";
  return false;
}

std::string var_string::to_string() {
  return "'" + value() + "'";
}

std::string var_string::defaultValue() const {
  return m_default_value;
}

std::string var_string::value() const {
  return m_value_init ? m_value : m_default_value;
}

void var_string::set_value(const std::string &val) {
  m_value = val;
  m_value_init = true;
}

// ---------------------------------------------------------------------
// ctf01d::var_bool

var_bool::var_bool(const std::vector<std::string> &path_name, bool default_value)
: ctf01d::var(path_name, ctf01d::var_type::BOOLEAN), m_default(default_value), m_value_init(false) {

}

// static
std::shared_ptr<var_bool> var_bool::create(
  const std::vector<std::string> &path_name,
  bool default_value,
  ctf01d::scope_vars &sc_vars
) {
  auto ret = std::make_shared<var_bool>(path_name, default_value);
  sc_vars.add_var(ret);
  return ret;
}

bool var_bool::read(WsjcppYamlCursor &cursor, std::string &err) {
  auto _cursor = cursor_by_path(cursor, err);
  if (_cursor.isValue()) {
    return set_value(_cursor.valBool(), err);
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
std::shared_ptr<var_dir> var_dir::create(
  const std::vector<std::string> &path_name,
  const std::string &default_value,
  const std::string &root_dir,
  ctf01d::scope_vars &sc_vars
) {
  auto ret = std::make_shared<var_dir>(path_name, default_value, root_dir);
  sc_vars.add_var(ret);
  return ret;
}

bool var_dir::read(WsjcppYamlCursor &cursor, std::string &err) {
  auto cur = cursor_by_path(cursor, err);
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
  m_absolute_path_value = to_absolute_path(m_value);
  m_absolute_path_default = to_absolute_path(m_default);
}

std::string var_dir::default_value() const {
  return m_default;
}

std::string var_dir::value() const {
  return m_value_init ? m_absolute_path_value : m_absolute_path_default;
}

bool var_dir::set_value(const std::string &val, std::string &err) {
  std::string new_val = to_absolute_path(val);

  if (!WsjcppCore::dirExists(new_val)) {
    err = "Directory '" + new_val + "' does not exists";
    WsjcppLog::err("var_dir", err);
    return false;
  }
  m_value = val;
  m_absolute_path_value = new_val;
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


// ---------------------------------------------------------------------
// ctf01d::var_file

var_file::var_file(const std::vector<std::string> &path_name, const std::string &default_value, const std::string &root_dir)
: ctf01d::var(path_name, ctf01d::var_type::STRING), m_value_init(false) {
  m_root_dir = root_dir;
  m_default = to_absolute_path(default_value);
}

// static
std::shared_ptr<var_file> var_file::create(
  const std::vector<std::string> &path_name,
  const std::string &default_value,
  const std::string &root_dir,
  ctf01d::scope_vars &sc_vars
) {
  auto ret = std::make_shared<var_file>(path_name, default_value, root_dir);
  sc_vars.add_var(ret);
  return ret;
}

bool var_file::read(WsjcppYamlCursor &cursor, std::string &err) {
  auto cur = cursor_by_path(cursor, err);
  if (cur.isValue()) {
    return set_value(cur.valStr(), err);
  }
  err = "var_file: Not value for '" + name() + "'";
  return false;
}

std::string var_file::to_string() {
  return "'" +  value() + "'";
}

void var_file::set_root_dir(const std::string &val) {
  m_root_dir = val;
  m_absolute_path_value = to_absolute_path(m_value);
  m_absolute_path_default = to_absolute_path(m_default);
}

std::string var_file::default_value() const {
  return m_default;
}

std::string var_file::value() const {
  return m_value_init ? m_absolute_path_value : m_absolute_path_default;
}

bool var_file::set_value(const std::string &val, std::string &err) {
  std::string new_val = to_absolute_path(val);

  if (!WsjcppCore::fileExists(new_val)) {
    err = "File '" + new_val + "' does not exists";
    WsjcppLog::err("var_file", err);
    return false;
  }
  m_value = val;
  m_absolute_path_value = new_val;
  m_value_init = true;
  return true;
}

std::string var_file::to_absolute_path(const std::string &val) {
  std::string ret = val;
  if (ret.size() > 0 && ret[0] != '/') {
    ret = m_root_dir + "/" + ret;
  }
  return wsjcpp::normalizeFilePath(ret);
}

// ---------------------------------------------------------------------
// ctf01d::var_datetime

var_datetime::var_datetime(const std::vector<std::string> &path_name, const std::string &default_value)
: ctf01d::var(path_name, ctf01d::var_type::STRING), m_value_init(false) {
  m_default = default_value;
  m_default_in_seconds = convert_to_seconds(m_default);
}

// static
std::shared_ptr<var_datetime> var_datetime::create(
  const std::vector<std::string> &path_name,
  const std::string &default_value,
  ctf01d::scope_vars &sc_vars
) {
  auto ret = std::make_shared<var_datetime>(path_name, default_value);
  sc_vars.add_var(ret);
  return ret;
}

bool var_datetime::read(WsjcppYamlCursor &cursor, std::string &err) {
  auto cur = cursor_by_path(cursor, err);
  if (cur.isValue()) {
    return set_value(cur.valStr(), err);
  }
  return false;
}

std::string var_datetime::to_string() {
  return "'" +  value() + "'";
}

std::string var_datetime::default_value() const {
  return m_default;
}

std::string var_datetime::value() const {
  return m_value_init ? m_value : m_default;
}

int var_datetime::value_in_seconds() const {
  return m_value_init ? m_value_in_seconds : m_default_in_seconds;
}

bool var_datetime::set_value(const std::string &val, std::string &err) {
  m_value = val;
  m_value_in_seconds = convert_to_seconds(m_value);
  m_value_init = true;
  return true;
}

int var_datetime::convert_to_seconds(const std::string &val)
{
  std::istringstream in{val.c_str()};
  date::sys_seconds tp;
  in >> date::parse("%Y-%m-%d %T", tp);
  return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
}

// ---------------------------------------------------------------------
// ctf01d::var_allowed_ip

var_ip_or_host::var_ip_or_host(const std::vector<std::string> &path_name)
: ctf01d::var(path_name, ctf01d::var_type::STRING), m_value_init(false) {
}

// static
std::shared_ptr<var_ip_or_host> var_ip_or_host::create(
  const std::vector<std::string> &path_name,
  ctf01d::scope_vars &sc_vars
) {
  auto ret = std::make_shared<var_ip_or_host>(path_name);
  sc_vars.add_var(ret);
  return ret;
}

bool var_ip_or_host::read(WsjcppYamlCursor &cursor, std::string &err) {
  auto cur = cursor_by_path(cursor, err);
  if (cur.isValue()) {
    return set_value(cur.valStr(), err);
  }
  return false;
}

std::string var_ip_or_host::to_string() {
  return "'" +  value() + "'";
}

void var_ip_or_host::set_prefix(const std::string &prefix) {
  m_prefix = prefix;
  // TODO validate
  m_final_value = m_prefix + m_value + m_suffix;
}

void var_ip_or_host::set_suffix(const std::string &suffix) {
  m_suffix = suffix;
  // TODO validate
  m_final_value = m_prefix + m_value + m_suffix;
}

std::string var_ip_or_host::value() const {
  return m_final_value;
}

bool var_ip_or_host::set_value(const std::string &val, std::string &err) {
  std::string final_value = m_prefix + val + m_suffix;
  // Check RFC length limit constraint (Max 255 characters total)
  if (final_value.empty()) {
    err = "IP or Hostname '" + final_value + "' could no be empty";
    return false;
  }

  if (final_value.length() > 255) {
    err = "IP or Hostname '" + final_value + "' could no be more than length 255";
    return false;
  }

  static const std::regex pattern_ipv4("^(\\d+\\.)+\\d+$");
  static const std::regex pattern_hostname(R"(^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9])$)");
  if (std::regex_match(final_value, pattern_ipv4)) {
    if (!is_valid_ip_v4(final_value, err)) {
      return false;
    }
  } else if (!std::regex_match(final_value, pattern_hostname)) {
    err = final_value + " is't could not be a host name";
    return false;
  }
    
  m_value = val;
  m_final_value = final_value;
  m_value_init = true;
  return true;
}

bool var_ip_or_host::is_valid_ip_v4(const std::string &value, std::string &err) {
  int n = 0;
  std::string s[4] = {"", "", "", ""};
  for (int i = 0; i < value.length(); i++) {
    char c = value[i];
    if (n > 3) {
      err = "Groups number must be less than 5 (like '0.0.0.0'), but got value " + value;
      return false;
    }
    if (c >= '0' && c <= '9') {
      s[n] += c;
    } else if (c == '.') {
      n++;
    } else {
      err = "Unexpected character '";
      err += c;
      err += "'";
      return false;
    }
  }
  for (int i = 0; i < 4; i++) {
    if (s[i].length() > 3) {
      err =
          "Value '" + s[i] + "' could not contains more than 3 digits in a row, but got value " + value;
      return false;
    }
    int p = std::stoi(s[i]);
    if (p > 255 || p < 0) {
      err = "Value '" + std::to_string(p) + "' must be 0..255, but got value " + value;
      return false;
    }
  }
  return true;
}

// ---------------------------------------------------------------------
// ctf01d::var_allowed_ip

var_allowed_ip::var_allowed_ip(const std::vector<std::string> &path_name, const std::string &default_value)
: ctf01d::var(path_name, ctf01d::var_type::STRING), m_value_init(false) {
  m_default = default_value;
}

// static
std::shared_ptr<var_allowed_ip> var_allowed_ip::create(
  const std::vector<std::string> &path_name,
  const std::string &default_value,
  ctf01d::scope_vars &sc_vars
) {
  auto ret = std::make_shared<var_allowed_ip>(path_name, default_value);
  sc_vars.add_var(ret);
  return ret;
}

bool var_allowed_ip::read(WsjcppYamlCursor &cursor, std::string &err) {
  auto cur = cursor_by_path(cursor, err);
  if (cur.isValue()) {
    return set_value(cur.valStr(), err);
  }
  return false;
}

std::string var_allowed_ip::to_string() {
  return "'" +  value() + "'";
}

std::string var_allowed_ip::default_value() const {
  return m_default;
}

std::string var_allowed_ip::value() const {
  return m_value_init ? m_value : m_default;
}

bool var_allowed_ip::set_value(const std::string &val, std::string &err) {
  m_value = val;
  m_value_init = true;
  return true;
}

} // namespace ctf01d
