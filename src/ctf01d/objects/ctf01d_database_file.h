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

#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include <string>

namespace sea5kg {

namespace sqlite3_wrapper {

class database_file;
class database_update;

class database_update_fabric_base {
public:
  virtual std::shared_ptr<database_update> create_update() = 0;
};

template <typename T> class database_update_fabric : public database_update_fabric_base {
public:
  virtual std::shared_ptr<database_update> create_update() override {
    return std::make_shared<T>();
  }
};

extern std::map<std::string, std::vector<std::shared_ptr<database_update_fabric_base>>> *g_database_updates_fabric;
extern std::map<std::string, database_file *> *g_opened_database_files;

class global {
public:
  static void registry_database_update_fabric(const std::string &db_name, std::shared_ptr<database_update_fabric_base>);
  static void add_opened_database_file(const std::string &name, database_file *db);
  static bool init_driver_sqlite3(int &ret);
  static void shutdown_driver_sqlite3();
};

class rows_iterator {
public:
  virtual bool next() = 0;
  virtual std::string as_string(int column_idx) = 0;
  virtual long as_long(int column_idx) = 0;
};

class database_file {
public:
  database_file(const std::string &db_name, const std::string &init_sql, const std::string &db_dir = "./",
                const std::string &filename = "", long backup_freq = 0);
  ~database_file();
  bool open(std::string &error);
  bool is_opened() const;
  void close();
  bool execute_query(const std::string &sql, std::string &error);
  int select_sum_or_count(const std::string &sql, std::string &error);
  std::shared_ptr<rows_iterator> select_rows(const std::string &sql, std::string &error);

private:
  bool copy_database_to_backup(std::string &error);
  std::mutex m_mutex;

  std::string TAG;
  void *m_db;
  std::string m_sFilename;
  std::string m_sFileFullpath;
  long m_backup_freq_in_seconds;
  std::string m_sBaseFileBackupFullpath;
  std::string m_init_sql;
  int m_last_backup_time;
};

} // namespace sqlite3_wrapper

} // namespace ctf01d