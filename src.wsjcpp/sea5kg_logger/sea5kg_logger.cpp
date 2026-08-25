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

#include "sea5kg_logger.h"

#ifndef _MSC_VER
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/time.h>
#include <unistd.h>
#else
#include <direct.h>
#define PATH_MAX 256
#endif

// #include <filesystem> // in c++ 17
#include <deque>
#include <fstream>
#include <iomanip> // std::setw
#include <iostream>
#include <mutex>
#include <sstream>
#include <sys/stat.h>
#include <thread>

namespace sea5kg {

long _current_time_in_milliseconds() {
  long nTimeStart =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
          .count();
  return nTimeStart;
}

long _current_time_in_seconds() {
  long nTimeStart =
      std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  return nTimeStart;
}

std::string _current_time_for_log_format() {
  long nTimeStart =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
          .count();
  std::string sMilliseconds = std::to_string(int(nTimeStart % 1000));
  nTimeStart = nTimeStart / 1000;

  std::time_t tm_ = long(nTimeStart);
  struct tm tstruct;
  gmtime_r(&tm_, &tstruct);

  // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
  // for more information about date/time format
  char buf[80];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
  return std::string(buf) + "." + std::string(3 - sMilliseconds.length(), '0').append(sMilliseconds);
}

std::string _get_thread_id() {
  static_assert(sizeof(std::thread::id) == sizeof(uint64_t),
                "this function only works if size of thead::id is equal to the size of uint_64");
  std::thread::id this_id = std::this_thread::get_id();
  uint64_t val = *((uint64_t *)&this_id);
  std::stringstream stream;
  stream << "0x" << std::setw(16) << std::setfill('0') << std::hex << val;
  return std::string(stream.str());
}

std::string _format_time_for_filename(long nTimeInSec) {
  std::time_t tm_ = long(nTimeInSec);
  // struct tm tstruct = *localtime(&tm_);
  struct tm tstruct;
  gmtime_r(&tm_, &tstruct);

  // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
  // for more information about date/time format
  char buf[80];
  strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tstruct);
  return std::string(buf);
}

enum class color_code {
  FG_0 = 0,
  FG_GRAY = 2,
  FG_RED = 31,
  FG_GREEN = 32,
  FG_BLUE = 34,
  FG_CYAN = 36,
  FG_DEFAULT = 39,
  FG_YELLOW = 93,
  BG_RED = 41,
  BG_GREEN = 42,
  BG_BLUE = 44,
  BG_CYAN = 46,
  BG_DEFAULT = 49
};

class color_modifier {
public:
  color_modifier(color_code _code) : code(_code) {
  }
  friend std::ostream &operator<<(std::ostream &os, const color_modifier &mod) {
    os << "\033[" << int(mod.code) << "m";
    return os;
  }

private:
  color_code code;
};

static sea5kg::color_modifier COLOR_DEFAULT(sea5kg::color_code::FG_0);
static sea5kg::color_modifier COLOR_YELLOW(sea5kg::color_code::FG_YELLOW);
static sea5kg::color_modifier COLOR_RED(sea5kg::color_code::FG_RED);
static sea5kg::color_modifier COLOR_GRAY(sea5kg::color_code::FG_GRAY);
static sea5kg::color_modifier COLOR_GREEN(sea5kg::color_code::FG_GREEN);

const sea5kg::color_modifier &log_level_to_color_modifier(log_level lvl) {
  switch (lvl) {
  case log_level::DEBUG:
    return COLOR_GRAY;
  case log_level::INFO:
    return COLOR_DEFAULT;
  case log_level::SUCCESS:
    return COLOR_GREEN;
  case log_level::WARNING:
    return COLOR_YELLOW;
  case log_level::ERROR:
    return COLOR_RED;
  case log_level::CRITICAL:
    return COLOR_RED;
  }
  return COLOR_DEFAULT;
}

class private_logger_impl : public logger {
public:
  private_logger_impl();
  virtual void set_log_dirpath(const std::string &log_dir) override;
  virtual const std::string &log_dirpath() override;
  virtual void set_log_filename_prefix(const std::string &prefix) override;
  virtual const std::string &log_file_fullpath() override;
  virtual void set_rotation_period_in_seconds(int val_in_seconds) override;
  virtual int rotation_period_in_seconds() override;
  virtual void set_log_level_file_output(log_level val) override;
  virtual log_level log_level_file_output() override;
  virtual void set_log_level_console_output(log_level val) override;
  virtual log_level log_level_console_output() override;
  virtual void set_log_level_redirect_to_global(log_level val) override;
  virtual log_level log_level_redirect_to_global() override;
  virtual void set_runtime_history_size(int val) override;
  virtual int runtime_history_size() override;
  virtual std::vector<std::string> runtime_history_messages() override;
  virtual void debug(const std::string &tag, const std::string &message) override;
  virtual void info(const std::string &tag, const std::string &message) override;
  virtual void success(const std::string &tag, const std::string &message) override;
  virtual void warning(const std::string &tag, const std::string &message) override;
  virtual void error(const std::string &tag, const std::string &message) override;
  virtual void critical(const std::string &tag, const std::string &message) override;

protected:
  void add(log_level log_level, const std::string &tag, const std::string &message);

private:
  void do_log_rotate_update_filename(log_level lvl);
  std::string prepare_log_dir(const std::string &log_dir, int t_now_seconds, log_level lvl);

  std::mutex m_mutex;
  std::string m_log_dir;
  std::string m_log_file_name_prefix;
  std::string m_log_file_fullpath;
  log_level m_log_level_file_output;
  log_level m_log_level_console_output;
  log_level m_log_level_redirect_to_global;
  int m_runtime_history_size;
  long m_log_start_time;
  int m_rotation_period_in_seconds;
  std::deque<std::string> m_runtime_history_messages;
};

private_logger_impl::private_logger_impl() {
  m_log_dir = "./";
  m_log_file_name_prefix = "";
  m_log_file_fullpath = "";
  m_log_level_file_output = log_level::DISABLE;
  m_log_level_console_output = log_level::DEBUG;
  m_log_level_redirect_to_global = log_level::DISABLE;
  m_log_start_time = 0;
  m_rotation_period_in_seconds = 86400; // 24h
  m_runtime_history_size = 0;
}

void private_logger_impl::set_log_dirpath(const std::string &log_dir) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_log_dir = log_dir;
  m_log_start_time = 0; // reset log rotate
  do_log_rotate_update_filename(m_log_level_file_output);
}

const std::string &private_logger_impl::log_dirpath() {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_log_dir;
}

void private_logger_impl::set_log_filename_prefix(const std::string &prefix) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_log_file_name_prefix = prefix;
  m_log_start_time = 0; // reset log rotate
  do_log_rotate_update_filename(m_log_level_file_output);
}

const std::string &private_logger_impl::log_file_fullpath() {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_log_file_fullpath;
}

void private_logger_impl::set_rotation_period_in_seconds(int val_in_seconds) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_rotation_period_in_seconds = val_in_seconds;
}

int private_logger_impl::rotation_period_in_seconds() {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_rotation_period_in_seconds;
}

void private_logger_impl::set_log_level_file_output(log_level val) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_log_level_file_output = val;
  m_log_start_time = 0; // reset log rotate
  do_log_rotate_update_filename(m_log_level_file_output);
}

log_level private_logger_impl::log_level_file_output() {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_log_level_file_output;
}

void private_logger_impl::set_log_level_console_output(log_level val) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_log_level_console_output = val;
}

log_level private_logger_impl::log_level_console_output() {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_log_level_console_output;
}

void private_logger_impl::set_log_level_redirect_to_global(log_level val) {
  if (this == log::g_GLOBAL) {
    log::warning("logger", "Forbidden set redirect from global log to global");
    return;
  }
  std::lock_guard<std::mutex> lock(m_mutex);
  m_log_level_redirect_to_global = val;
}

log_level private_logger_impl::log_level_redirect_to_global() {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_log_level_redirect_to_global;
}

void private_logger_impl::set_runtime_history_size(int val) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_runtime_history_size = val;
}

int private_logger_impl::runtime_history_size() {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_runtime_history_size;
}

std::vector<std::string> private_logger_impl::runtime_history_messages() {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<std::string> ret;
  for (int i = 0; i < m_runtime_history_messages.size(); i++) {
    ret.push_back(m_runtime_history_messages[i]);
  }
  return ret;
}

void private_logger_impl::debug(const std::string &tag, const std::string &message) {
  add(log_level::DEBUG, tag, message);
}

void private_logger_impl::info(const std::string &tag, const std::string &message) {
  add(log_level::INFO, tag, message);
}

void private_logger_impl::success(const std::string &tag, const std::string &message) {
  add(log_level::SUCCESS, tag, message);
}

void private_logger_impl::warning(const std::string &tag, const std::string &message) {
  add(log_level::WARNING, tag, message);
}

void private_logger_impl::error(const std::string &tag, const std::string &message) {
  add(log_level::ERROR, tag, message);
}

void private_logger_impl::critical(const std::string &tag, const std::string &message) {
  add(log_level::CRITICAL, tag, message);
  throw std::runtime_error(message);
}

void private_logger_impl::do_log_rotate_update_filename(log_level lvl) {
  if (lvl == log_level::DISABLE) {
    return;
  }
  int t_now_seconds =
      std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  long rotate_diff = t_now_seconds - m_log_start_time;
  if (m_log_start_time == 0 || rotate_diff > m_rotation_period_in_seconds) {
    m_log_start_time = t_now_seconds;
    std::string log_dir_formatted = prepare_log_dir(m_log_dir, t_now_seconds, lvl);
    m_log_file_fullpath =
        log_dir_formatted + "/" + m_log_file_name_prefix + _format_time_for_filename(m_log_start_time) + ".log";
  }
}

bool _dir_exists(const std::string &sDirname) {
  struct stat st;
  bool bExists = (stat(sDirname.c_str(), &st) == 0);
  if (bExists) {
    return (st.st_mode & S_IFDIR) != 0;
  }
  return false;
}

bool _make_dir(const std::string &dir_name) {
  if (_dir_exists(dir_name)) {
    return true;
  }

  struct stat st;

  // const std::filesystem::path dir{dir_name};
  // std::filesystem::create_directory(dir);

  int nStatus = mkdir(dir_name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if (nStatus == 0) {
    return true;
  }
  if (nStatus == EACCES) {
    std::cout << " " << dir_name << std::endl;
    throw std::runtime_error("FAILED create folder '" + dir_name + "'");
    return false;
  }
  return true;
}

std::string private_logger_impl::prepare_log_dir(const std::string &log_dir, int t_now_seconds, log_level lvl) {
  std::string log_dir_formatted = log_dir;
  {
    std::time_t tm_ = long(t_now_seconds);
    // struct tm tstruct = *localtime(&tm_);
    struct tm tstruct;
    gmtime_r(&tm_, &tstruct);

    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    char buf[255];
    strftime(buf, sizeof(buf), log_dir_formatted.c_str(), &tstruct);
    log_dir_formatted = std::string(buf);
  }
  if (lvl >= m_log_level_file_output) {
    if (!_dir_exists(log_dir_formatted)) {
      // make dirs
      // std::cout << "Make dirs for log: " << log_dir_formatted << std::endl;
      std::string sub_path = "";
      for (int i = 0; i < log_dir_formatted.size(); ++i) {
        if (log_dir_formatted[i] == '/') {
          _make_dir(sub_path);
        }
        sub_path += log_dir_formatted[i];
      }
      _make_dir(sub_path);
      if (!_dir_exists(log_dir_formatted)) {
        throw std::runtime_error("FAILED create folder '" + log_dir_formatted + "'");
      }
    }
  }
  return log_dir_formatted;
}

static const std::string type_none = " [?] ";
static const std::string type_debug = " [DEBUG] ";
static const std::string type_info = " [INFO] ";
static const std::string type_success = " [SUCCESS] ";
static const std::string type_warning = " [WARNING] ";
static const std::string type_error = " [ERROR] ";
static const std::string type_critical = " [CRITICAL] ";

const std::string &log_level_to_type(log_level lvl) {
  switch (lvl) {
  case log_level::DEBUG:
    return type_debug;
  case log_level::INFO:
    return type_info;
  case log_level::SUCCESS:
    return type_success;
  case log_level::WARNING:
    return type_warning;
  case log_level::ERROR:
    return type_error;
  case log_level::CRITICAL:
    return type_critical;
  }
  return type_none;
}

void init_log_message(std::string &log_message, log_level lvl, const std::string &tag, const std::string &message) {
  if (log_message.empty()) {
    log_message =
        _current_time_for_log_format() + ", " + _get_thread_id() + log_level_to_type(lvl) + tag + ": " + message;
  }
}

void private_logger_impl::add(log_level lvl, const std::string &tag, const std::string &message) {
  std::lock_guard<std::mutex> lock(m_mutex);
  do_log_rotate_update_filename(lvl);

  std::string log_message;
  if (lvl >= m_log_level_console_output) {
    init_log_message(log_message, lvl, tag, message);
    std::cout << log_level_to_color_modifier(lvl) << log_message << COLOR_DEFAULT << std::endl;
  }

  if (lvl >= m_log_level_file_output) {
    std::ofstream log_file(m_log_file_fullpath, std::ios::app);
    if (!log_file) {
      std::cerr << "Error Opening File '" << m_log_file_fullpath << "'" << std::endl;
      return;
    }
    init_log_message(log_message, lvl, tag, message);
    log_file << log_message << std::endl;
    log_file.close();
  }

  if (m_runtime_history_size > 0) {
    init_log_message(log_message, lvl, tag, message);
    m_runtime_history_messages.push_front(log_message);
    while (m_runtime_history_messages.size() > m_runtime_history_size) {
      m_runtime_history_messages.pop_back();
    }
  }

  if (this != log::g_GLOBAL && lvl >= m_log_level_redirect_to_global) {
    ((private_logger_impl *)log::g_GLOBAL)->add(lvl, tag, message);
  }
}

logger *logger::create() {
  return new private_logger_impl();
}

logger *log::g_GLOBAL = logger::create();

void log::set_log_dirpath(const std::string &log_dir) {
  log::g_GLOBAL->set_log_dirpath(log_dir);
}

const std::string &log::log_dirpath() {
  return log::g_GLOBAL->log_dirpath();
}

void log::set_log_filename_prefix(const std::string &prefix) {
  log::g_GLOBAL->set_log_filename_prefix(prefix);
}

void log::set_log_level_file_output(log_level val) {
  log::g_GLOBAL->set_log_level_file_output(val);
}

log_level log::log_level_file_output() {
  return log::g_GLOBAL->log_level_file_output();
}

void log::set_log_level_console_output(log_level val) {
  log::g_GLOBAL->set_log_level_console_output(val);
}

log_level log::log_level_console_output() {
  return log::g_GLOBAL->log_level_console_output();
}

void log::set_rotation_period_in_seconds(int val_in_seconds) {
  log::g_GLOBAL->set_rotation_period_in_seconds(val_in_seconds);
}

int log::rotation_period_in_seconds() {
  return log::g_GLOBAL->rotation_period_in_seconds();
}

void log::set_runtime_history_size(int val) {
  log::g_GLOBAL->set_runtime_history_size(val);
}

int log::runtime_history_size() {
  return log::g_GLOBAL->runtime_history_size();
}

std::vector<std::string> log::runtime_history_messages() {
  return log::g_GLOBAL->runtime_history_messages();
}

void log::debug(const std::string &tag, const std::string &message) {
  log::g_GLOBAL->debug(tag, message);
}

void log::info(const std::string &tag, const std::string &message) {
  log::g_GLOBAL->info(tag, message);
}

void log::success(const std::string &tag, const std::string &message) {
  log::g_GLOBAL->success(tag, message);
}

void log::warning(const std::string &tag, const std::string &message) {
  log::g_GLOBAL->warning(tag, message);
}

void log::error(const std::string &tag, const std::string &message) {
  log::g_GLOBAL->error(tag, message);
}

void log::critical(const std::string &tag, const std::string &message) {
  log::g_GLOBAL->critical(tag, message);
}

} // namespace sea5kg
