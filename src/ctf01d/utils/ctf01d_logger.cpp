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

#include "ctf01d_logger.h"
#include <wsjcpp_core.h>
#include <iostream>
#include <sstream>
#include <fstream>

namespace ctf01d {

enum class color_code {
  FG_RED      = 31,
  FG_GREEN    = 32,
  FG_YELLOW   = 93,
  FG_BLUE     = 34,
  FG_DEFAULT  = 39,
  BG_RED      = 41,
  BG_GREEN    = 42,
  BG_BLUE     = 44,
  BG_DEFAULT  = 49
};

class color_modifier {
ctf01d::color_code code;
public:
  color_modifier(ctf01d::color_code pCode) : code(pCode) {}
  friend std::ostream &operator<<(std::ostream& os, const ctf01d::color_modifier &mod) {
    os << "\033[" << int(mod.code) << "m";
    return os;
  }
};

ctf01d::color_modifier RED(ctf01d::color_code::FG_RED);

class private_logger_impl : public ctf01d::logger {
public:
  private_logger_impl();
  virtual void set_log_dirpath(const std::string &log_dir) override;
  virtual const std::string &get_log_dirpath() override;
  virtual void set_log_filename_prefix(const std::string &prefix) override;
  virtual const std::string &get_log_file_fullpath() override;
  virtual void set_rotation_period_in_seconds(int val_in_seconds) override;
  virtual int get_rotation_period_in_seconds() override;
  virtual bool get_enable_log_file() override;
  virtual void set_enable_log_file(bool val) override;
  virtual bool get_enable_console_output() override;
  virtual void set_enable_console_output(bool val) override;
  virtual void info(const std::string &tag, const std::string &message) override;
  virtual void err(const std::string &tag, const std::string &message) override;
  virtual void throw_err(const std::string &tag, const std::string &message) override;
  virtual void warn(const std::string &tag, const std::string &message) override;
  virtual void ok(const std::string &tag, const std::string &message) override;
  
private:
  void do_log_rotate_update_filename(bool force = false);
  void add(ctf01d::color_modifier &clr, const std::string &sType, const std::string &tag, const std::string &message);

  std::mutex m_mutex;
  std::string m_log_dir;
  std::string m_log_file_name_prefix;
  std::string m_log_file_fullpath;
  bool m_enable_log_file;
  bool m_enable_console_output;
  long m_log_start_time;
  int m_rotation_period_in_seconds;
};

private_logger_impl::private_logger_impl() {
  m_log_dir = "./";
  m_log_file_name_prefix = "";
  m_log_file_fullpath = "";
  m_enable_log_file = false;
  m_enable_console_output = true;
  m_log_start_time = 0;
  m_rotation_period_in_seconds = 86400; // 24h
}

void private_logger_impl::set_log_dirpath(const std::string &log_dir) {
  m_log_dir = log_dir;
  if (m_enable_log_file) {
    if (!WsjcppCore::dirExists(m_log_dir)) {
      if (!WsjcppCore::makeDir(m_log_dir)) {
        log::throw_err("set_log_directory", "Could not create log directory '" + m_log_dir + "'");
      }
    }
  }
  do_log_rotate_update_filename(true);
}

const std::string &private_logger_impl::get_log_dirpath() {
  return m_log_dir;
}

void private_logger_impl::set_log_filename_prefix(const std::string &prefix) {
  m_log_file_name_prefix = prefix;
  do_log_rotate_update_filename(true);
}

const std::string &private_logger_impl::get_log_file_fullpath() {
  return m_log_file_fullpath;
}

void private_logger_impl::set_rotation_period_in_seconds(int val_in_seconds) {
  m_rotation_period_in_seconds = val_in_seconds;
}

int private_logger_impl::get_rotation_period_in_seconds() {
  return m_rotation_period_in_seconds;
}

bool private_logger_impl::get_enable_log_file() {
  return m_enable_log_file;
}

void private_logger_impl::set_enable_log_file(bool val) {
  m_enable_log_file = val;
  // make a log dir
  if (m_enable_log_file) {
    if (!WsjcppCore::dirExists(m_log_dir)) {
      if (!WsjcppCore::makeDir(m_log_dir)) {
        log::throw_err("set_enable_log_file", "Could not create log directory '" + m_log_dir + "'");
      }
    }
  }
  do_log_rotate_update_filename(true);
}

bool private_logger_impl::get_enable_console_output() {
  return m_enable_console_output;
}

void private_logger_impl::set_enable_console_output(bool val) {
  m_enable_console_output = val;
}

void private_logger_impl::info(const std::string &tag, const std::string &message) {
  ctf01d::color_modifier def(ctf01d::color_code::FG_DEFAULT);
  add(def, "INFO", tag, message);
}

void private_logger_impl::err(const std::string &tag, const std::string &message) {
  ctf01d::color_modifier red(ctf01d::color_code::FG_RED);
  add(red, "ERR", tag, message);
}

void private_logger_impl::throw_err(const std::string &tag, const std::string &message) {
  add(RED, "ERR", tag, message);
  throw std::runtime_error(message);
}

void private_logger_impl::warn(const std::string &tag, const std::string &message) {
  ctf01d::color_modifier yellow(ctf01d::color_code::FG_YELLOW);
  add(yellow, "WARN",tag, message);
}

void private_logger_impl::ok(const std::string &tag, const std::string &message) {
  ctf01d::color_modifier green(ctf01d::color_code::FG_GREEN);
  add(green, "OK", tag, message);
}

void private_logger_impl::do_log_rotate_update_filename(bool force) {
  long t_now_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  long rotate_diff = t_now_seconds - m_log_start_time;
  if (force || m_log_start_time == 0 || rotate_diff > m_rotation_period_in_seconds) {
    m_log_start_time = t_now_seconds;
    m_log_file_fullpath = m_log_dir + "/"
      + m_log_file_name_prefix + "_"
      + WsjcppCore::formatTimeForFilename(m_log_start_time) + ".log";
  }
}

void private_logger_impl::add(ctf01d::color_modifier &clr, const std::string &sType, const std::string &tag, const std::string &message) {
  do_log_rotate_update_filename();

  std::lock_guard<std::mutex> lock(m_mutex);
  ctf01d::color_modifier def(ctf01d::color_code::FG_DEFAULT);

  std::string sLogMessage = WsjcppCore::getCurrentTimeForLogFormat() + ", " + WsjcppCore::getThreadId()
    + " [" + sType + "] " + tag + ": " + message;
  if (m_enable_console_output) {
    std::cout << clr << sLogMessage << def << std::endl;
  }

  // log file
  if (m_enable_log_file) {
    std::ofstream log_file(m_log_file_fullpath, std::ios::app);
    if (!log_file) {
        std::cout << "Error Opening File" << std::endl;
        return;
    }

    log_file << sLogMessage << std::endl;
    log_file.close();
  }
}

logger *logger::create() {
  return new ctf01d::private_logger_impl();
}

ctf01d::logger *log::g_WSJCPP_LOG_GLOBAL_CONF = logger::create();

void log::info(const std::string &tag, const std::string &message) {
  log::g_WSJCPP_LOG_GLOBAL_CONF->info(tag, message);
}

void log::err(const std::string &tag, const std::string &message) {
  log::g_WSJCPP_LOG_GLOBAL_CONF->err(tag, message);
}

void log::throw_err(const std::string &tag, const std::string &message) {
  log::g_WSJCPP_LOG_GLOBAL_CONF->throw_err(tag, message);
}

void log::warn(const std::string & tag, const std::string &message) {
  log::g_WSJCPP_LOG_GLOBAL_CONF->warn(tag, message);
}

void log::ok(const std::string &tag, const std::string &message) {
  log::g_WSJCPP_LOG_GLOBAL_CONF->ok(tag, message);
}

void log::set_log_dirpath(const std::string &log_dir) {
  log::g_WSJCPP_LOG_GLOBAL_CONF->set_log_dirpath(log_dir);
}

const std::string &log::get_log_dirpath() {
  return log::g_WSJCPP_LOG_GLOBAL_CONF->get_log_dirpath();
}

void log::set_log_filename_prefix(const std::string &prefix) {
  log::g_WSJCPP_LOG_GLOBAL_CONF->set_log_filename_prefix(prefix);
}

void log::set_enable_log_file(bool val) {
  log::g_WSJCPP_LOG_GLOBAL_CONF->set_enable_log_file(val);
}

void log::set_rotation_period_in_seconds(int val_in_seconds) {
  log::g_WSJCPP_LOG_GLOBAL_CONF->set_rotation_period_in_seconds(val_in_seconds);
}

int log::get_rotation_period_in_seconds() {
  return log::g_WSJCPP_LOG_GLOBAL_CONF->get_rotation_period_in_seconds();
}


} // namespace ctf01d