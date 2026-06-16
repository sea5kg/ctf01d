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
    #include <dirent.h>
    #include <sys/time.h>
    #include <unistd.h>
    #include <arpa/inet.h>
#else
    #include <direct.h>
    #define PATH_MAX 256
#endif

// #include <filesystem> // in c++ 17
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <iomanip> // std::setw
#include <mutex>
#include <deque>

namespace sea5kg {

long _current_time_in_milliseconds() {
  long nTimeStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  return nTimeStart;
}

long _current_time_in_seconds() {
  long nTimeStart = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  return nTimeStart;
}

std::string _current_time_for_log_format() {
  long nTimeStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  std::string sMilliseconds = std::to_string(int(nTimeStart % 1000));
  nTimeStart = nTimeStart / 1000;

  std::time_t tm_ = long(nTimeStart);
  struct tm tstruct;
  gmtime_r(&tm_, &tstruct);

  // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
  // for more information about date/time format
  char buf[80];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
  return std::string(buf) + "." + std::string( 3 - sMilliseconds.length(), '0').append(sMilliseconds);
}

std::string _get_thread_id() {
  static_assert(sizeof(std::thread::id)==sizeof(uint64_t),"this function only works if size of thead::id is equal to the size of uint_64");
  std::thread::id this_id = std::this_thread::get_id();
  uint64_t val = *((uint64_t*) &this_id);
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

bool _dir_exists(const std::string &sDirname) {
  struct stat st;
  bool bExists = (stat(sDirname.c_str(), &st) == 0);
  if (bExists) {
    return (st.st_mode & S_IFDIR) != 0;
  }
  return false;
}

bool _make_dir(const std::string &dir_name) {
  struct stat st;

  // const std::filesystem::path dir{dir_name};
  // std::filesystem::create_directory(dir);

  int nStatus = mkdir(dir_name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if (nStatus == 0) {
    return true;
  }
  if (nStatus == EACCES) {
    std::cout << "FAILED create folder " << dir_name << std::endl;
    return false;
  }
  return true;
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
  color_modifier(color_code _code) : code(_code) {}
  friend std::ostream &operator<<(std::ostream& os, const color_modifier &mod) {
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

class private_logger_impl : public logger {
public:
  private_logger_impl();
  virtual void set_log_dirpath(const std::string &log_dir) override;
  virtual const std::string &get_log_dirpath() override;
  virtual void set_log_filename_prefix(const std::string &prefix) override;
  virtual const std::string &get_log_file_fullpath() override;
  virtual void set_rotation_period_in_seconds(int val_in_seconds) override;
  virtual int rotation_period_in_seconds() override;
  virtual void set_enable_log_file(bool val) override;
  virtual bool enable_log_file() override;
  virtual void set_enable_console_output(bool val) override;
  virtual bool enable_console_output() override;
  virtual void set_runtime_history_size(int val) override;
  virtual int runtime_history_size() override;
  virtual std::vector<std::string> runtime_history_messages() override;
  virtual void debug(const std::string &tag, const std::string &message) override;
  virtual void info(const std::string &tag, const std::string &message) override;
  virtual void err(const std::string &tag, const std::string &message) override;
  virtual void throw_err(const std::string &tag, const std::string &message) override;
  virtual void warn(const std::string &tag, const std::string &message) override;
  virtual void ok(const std::string &tag, const std::string &message) override;

private:
  void do_log_rotate_update_filename(bool force = false);
  void add(color_modifier &clr, const std::string &type, const std::string &tag, const std::string &message);

  std::mutex m_mutex;
  std::string m_log_dir;
  std::string m_log_file_name_prefix;
  std::string m_log_file_fullpath;
  bool m_enable_log_file;
  bool m_enable_console_output;
  int m_runtime_history_size;
  long m_log_start_time;
  int m_rotation_period_in_seconds;
  std::deque<std::string> m_runtime_history_messages;
};

private_logger_impl::private_logger_impl() {
  m_log_dir = "./";
  m_log_file_name_prefix = "";
  m_log_file_fullpath = "";
  m_enable_log_file = false;
  m_enable_console_output = true;
  m_log_start_time = 0;
  m_rotation_period_in_seconds = 86400; // 24h
  m_runtime_history_size = 0;
}

void private_logger_impl::set_log_dirpath(const std::string &log_dir) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_log_dir = log_dir;
  if (m_enable_log_file) {
    if (!_dir_exists(m_log_dir)) {
      if (!_make_dir(m_log_dir)) {
        log::throw_err("set_log_dirpath", "Could not create log directory '" + m_log_dir + "'");
      }
    }
  }
  do_log_rotate_update_filename(true);
}

const std::string &private_logger_impl::get_log_dirpath() {
  return m_log_dir;
}

void private_logger_impl::set_log_filename_prefix(const std::string &prefix) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_log_file_name_prefix = prefix;
  do_log_rotate_update_filename(true);
}

const std::string &private_logger_impl::get_log_file_fullpath() {
  return m_log_file_fullpath;
}

void private_logger_impl::set_rotation_period_in_seconds(int val_in_seconds) {
  m_rotation_period_in_seconds = val_in_seconds;
}

int private_logger_impl::rotation_period_in_seconds() {
  return m_rotation_period_in_seconds;
}

void private_logger_impl::set_enable_log_file(bool val) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_enable_log_file = val;
  // make a log dir
  if (m_enable_log_file) {
    if (!_dir_exists(m_log_dir)) {
      if (!_make_dir(m_log_dir)) {
        log::throw_err("set_enable_log_file", "Could not create log directory '" + m_log_dir + "'");
      }
    }
  }
  do_log_rotate_update_filename(true);
}

bool private_logger_impl::enable_log_file() {
  return m_enable_log_file;
}

void private_logger_impl::set_enable_console_output(bool val) {
  m_enable_console_output = val;
}

bool private_logger_impl::enable_console_output() {
  return m_enable_console_output;
}

void private_logger_impl::set_runtime_history_size(int val) {
  m_runtime_history_size = val;
}

int private_logger_impl::runtime_history_size() {
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
  add(COLOR_GRAY, "DEBUG", tag, message);
}

void private_logger_impl::info(const std::string &tag, const std::string &message) {
  add(COLOR_DEFAULT, "INFO", tag, message);
}

void private_logger_impl::err(const std::string &tag, const std::string &message) {
  add(COLOR_RED, "ERR", tag, message);
}

void private_logger_impl::throw_err(const std::string &tag, const std::string &message) {
  add(COLOR_RED, "ERR", tag, message);
  throw std::runtime_error(message);
}

void private_logger_impl::warn(const std::string &tag, const std::string &message) {
  add(COLOR_YELLOW, "WARN",tag, message);
}

void private_logger_impl::ok(const std::string &tag, const std::string &message) {
  add(COLOR_GREEN, "OK", tag, message);
}

void private_logger_impl::do_log_rotate_update_filename(bool force) {
  long t_now_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  long rotate_diff = t_now_seconds - m_log_start_time;
  if (force || m_log_start_time == 0 || rotate_diff > m_rotation_period_in_seconds) {
    m_log_start_time = t_now_seconds;
    m_log_file_fullpath = m_log_dir + "/"
      + m_log_file_name_prefix
      + _format_time_for_filename(m_log_start_time) + ".log";
  }
}

void private_logger_impl::add(color_modifier &clr, const std::string &type, const std::string &tag, const std::string &message) {
  std::lock_guard<std::mutex> lock(m_mutex);
  do_log_rotate_update_filename();

  std::string log_message = _current_time_for_log_format() + ", " + _get_thread_id() + " [" + type + "] " + tag + ": " + message;
  if (m_enable_console_output) {
    std::cout << clr << log_message << COLOR_DEFAULT << std::endl;
  }
  
  if (m_runtime_history_size > 0) {
    m_runtime_history_messages.push_front(log_message);
    while (m_runtime_history_messages.size() > m_runtime_history_size) {
      m_runtime_history_messages.pop_back();
    }
  }
  if (m_enable_log_file) {
    std::ofstream log_file(m_log_file_fullpath, std::ios::app);
    if (!log_file) {
      std::cout << "Error Opening File" << std::endl;
      return;
    }
    log_file << log_message << std::endl;
    log_file.close();
  }
}

logger *logger::create() {
  return new private_logger_impl();
}

logger *log::g_GLOBAL = logger::create();

void log::debug(const std::string &tag, const std::string &message) {
  log::g_GLOBAL->debug(tag, message);
}

void log::info(const std::string &tag, const std::string &message) {
  log::g_GLOBAL->info(tag, message);
}

void log::err(const std::string &tag, const std::string &message) {
  log::g_GLOBAL->err(tag, message);
}

void log::throw_err(const std::string &tag, const std::string &message) {
  log::g_GLOBAL->throw_err(tag, message);
}

void log::warn(const std::string & tag, const std::string &message) {
  log::g_GLOBAL->warn(tag, message);
}

void log::ok(const std::string &tag, const std::string &message) {
  log::g_GLOBAL->ok(tag, message);
}

void log::set_log_dirpath(const std::string &log_dir) {
  log::g_GLOBAL->set_log_dirpath(log_dir);
}

const std::string &log::get_log_dirpath() {
  return log::g_GLOBAL->get_log_dirpath();
}

void log::set_log_filename_prefix(const std::string &prefix) {
  log::g_GLOBAL->set_log_filename_prefix(prefix);
}

void log::set_enable_log_file(bool val) {
  log::g_GLOBAL->set_enable_log_file(val);
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

} // namespace sea5kg
