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
#include <mutex>
#include <vector>
#include <iostream>
#include <deque>

namespace ctf01d {

class logger {
public:
  static logger *create();
  virtual void set_log_dirpath(const std::string &log_dir) = 0;
  virtual const std::string &get_log_dirpath() = 0;
  virtual void set_log_filename_prefix(const std::string &prefix) = 0;
  virtual const std::string &get_log_file_fullpath() = 0;
  virtual void set_rotation_period_in_seconds(int val_in_seconds) = 0;
  virtual int get_rotation_period_in_seconds() = 0;
  virtual bool get_enable_log_file() = 0;
  virtual void set_enable_log_file(bool val) = 0;
  virtual void info(const std::string &tag, const std::string &message) = 0;
  virtual void err(const std::string &tag, const std::string &message) = 0;
  virtual void throw_err(const std::string &tag, const std::string &message) = 0;
  virtual void warn(const std::string &tag, const std::string &message) = 0;
  virtual void ok(const std::string &tag, const std::string &message) = 0;
};

class log {
public:
  static ctf01d::logger *g_WSJCPP_LOG_GLOBAL_CONF;
  static void info(const std::string &tag, const std::string &message);
  static void err(const std::string &tag, const std::string &message);
  static void throw_err(const std::string &tag, const std::string &message);
  static void warn(const std::string &tag, const std::string &message);
  static void ok(const std::string &tag, const std::string &message);
  static void set_log_dirpath(const std::string &log_dir);
  static const std::string &get_log_dirpath();
  static void set_log_filename_prefix(const std::string &prefix);
  static void set_enable_log_file(bool val);
  static void set_rotation_period_in_seconds(int val_in_seconds);
  static int get_rotation_period_in_seconds();
};

} // namespace ctf01d