/*
 * MIT License
 *
 * Copyright (c) 2019-2026 Evgenii Sopov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// original source-code: https://github.com/sea5kg/sea5kg-logger

#ifndef SEA5KG_LOGGER_H
#define SEA5KG_LOGGER_H

#define SEA5KG_LOGGER_VER "2.0.1"
#define SEA5KG_LOGGER_VER_MAJOR "2"
#define SEA5KG_LOGGER_VER_MINOR "0"
#define SEA5KG_LOGGER_VER_BUILD "1"
#define SEA5KG_LOGGER_VER_2
#define SEA5KG_LOGGER_VER_2_0
#define SEA5KG_LOGGER_VER_2_0_1

#include <string>
#include <vector>

namespace sea5kg {

enum class log_level {
  DEBUG = 0,
  INFO = 10,
  SUCCESS = 20,
  WARNING = 30,
  ERROR = 40,
  CRITICAL = 50,
  DISABLE = 60,
};

class logger {
public:
  static logger *create();
  virtual void set_log_dirpath(const std::string &log_dir) = 0;
  virtual const std::string &log_dirpath() = 0;
  virtual void set_log_filename_prefix(const std::string &prefix) = 0;
  virtual const std::string &log_file_fullpath() = 0;
  virtual void set_rotation_period_in_seconds(int val_in_seconds) = 0;
  virtual int rotation_period_in_seconds() = 0;
  virtual void set_log_level_file_output(log_level val) = 0;
  virtual log_level log_level_file_output() = 0;
  virtual void set_log_level_console_output(log_level val) = 0;
  virtual log_level log_level_console_output() = 0;
  virtual void set_log_level_redirect_to_global(log_level val) = 0;
  virtual log_level log_level_redirect_to_global() = 0;
  virtual void set_runtime_history_size(int val) = 0;
  virtual int runtime_history_size() = 0;
  virtual std::vector<std::string> runtime_history_messages() = 0;
  virtual void debug(const std::string &tag, const std::string &message) = 0;
  virtual void info(const std::string &tag, const std::string &message) = 0;
  virtual void success(const std::string &tag, const std::string &message) = 0;
  virtual void warning(const std::string &tag, const std::string &message) = 0;
  virtual void error(const std::string &tag, const std::string &message) = 0;
  virtual void critical(const std::string &tag, const std::string &message) = 0;
};

class log {
public:
  static sea5kg::logger *g_GLOBAL;
  static void set_log_dirpath(const std::string &dirpath);
  static const std::string &log_dirpath();
  static void set_log_filename_prefix(const std::string &prefix);
  static void set_log_level_file_output(log_level val);
  static log_level log_level_file_output();
  static void set_log_level_console_output(log_level val);
  static log_level log_level_console_output();
  static void set_rotation_period_in_seconds(int val_in_seconds);
  static int rotation_period_in_seconds();
  static void set_runtime_history_size(int val);
  static int runtime_history_size();
  static std::vector<std::string> runtime_history_messages();
  static void debug(const std::string &tag, const std::string &message);
  static void info(const std::string &tag, const std::string &message);
  static void success(const std::string &tag, const std::string &message);
  static void warning(const std::string &tag, const std::string &message);
  static void error(const std::string &tag, const std::string &message);
  static void critical(const std::string &tag, const std::string &message);
};

} // namespace sea5kg

#endif // SEA5KG_LOGGER_H
