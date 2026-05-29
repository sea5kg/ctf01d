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
#include <vector>
#include <memory>
#include <wsjcpp_yaml.h>

namespace ctf01d {

enum class var_type {
  UNKNOWN = 0,
  INTEGER = 1,
  STRING = 2,
  BOOLEAN = 3,
  DATETIME = 4,
};

class var {
public:
  var(const std::vector<std::string> &path_name, ctf01d::var_type t);
  std::string name() const;
  ctf01d::var_type type() const;
  virtual bool read(WsjcppYaml &yaml, std::string &err);
  virtual std::string to_string();

protected:
  WsjcppYamlCursor cursorByPath(WsjcppYaml &yaml, std::string &err);

  std::vector<std::string> m_path_name;
  std::string m_name;
  ctf01d::var_type m_t;
};

class var_int : public ctf01d::var {
public:
  var_int(const std::vector<std::string> &path_name, int default_value);
  static std::shared_ptr<var_int> create(const std::vector<std::string> &path_name, int default_value);
  virtual bool read(WsjcppYaml &yaml, std::string &err) override;
  virtual std::string to_string() override;
  int default_value() const;
  int value() const;
  bool set_value(int val, std::string &err);
  void set_minimum(int val);
  void set_maximum(int val);

private:
  int m_default;
  bool m_value_init;
  int m_value;
  int m_minimum;
  bool m_check_minimum;
  int m_maximum;
  bool m_check_maximum;
};

class var_string : public ctf01d::var {
public:
  var_string(const std::vector<std::string> &path_name, const std::string &default_value);
  static std::shared_ptr<var_string> create(const std::vector<std::string> &path_name, const std::string &default_value);
  virtual bool read(WsjcppYaml &yaml, std::string &err) override;
  virtual std::string to_string() override;
  std::string defaultValue() const;
  std::string value() const;
  void setValue(const std::string &val);

private:
  std::string m_default_value;
  bool m_value_init;
  std::string m_value;
};

class var_bool : public ctf01d::var {
public:
  var_bool(const std::vector<std::string> &path_name, bool default_value);
  static std::shared_ptr<var_bool> create(const std::vector<std::string> &path_name, bool default_value);
  virtual bool read(WsjcppYaml &yaml, std::string &err) override;
  virtual std::string to_string() override;
  bool default_value() const;
  bool value() const;
  bool set_value(bool val, std::string &err);

private:
  bool m_default;
  bool m_value_init;
  bool m_value;
};

class var_dir : public ctf01d::var {
public:
  var_dir(const std::vector<std::string> &path_name, const std::string &default_value, const std::string &root_dir);
  static std::shared_ptr<var_dir> create(const std::vector<std::string> &path_name, const std::string &default_value, const std::string &root_dir);
  virtual bool read(WsjcppYaml &yaml, std::string &err) override;
  virtual std::string to_string() override;
  void set_root_dir(const std::string &val);
  std::string default_value() const;
  std::string value() const;
  bool set_value(const std::string &val, std::string &err);

private:
  std::string to_absolute_path(const std::string &val);
  std::string m_default;
  std::string m_absolute_path_default;
  bool m_value_init;
  std::string m_value;
  std::string m_absolute_path_value;
  std::string m_root_dir;
};

} // namespace ctf01d
