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
#include <string>

namespace ctf01d {

class database_file;

extern std::map<std::string, database_file *> *g_opened_database_files;

class global_databases {
public:
  static void add_opened_database_file(const std::string &name, database_file *db);
  static bool init_driver_sqlite3(int &ret);
  static void shutdown_driver_sqlite3();
};

class database_select_rows {
public:
  virtual bool next() = 0;
  virtual std::string getString(int nColumnNumber) = 0;
  virtual long getLong(int nColumnNumber) = 0;
};

class database_file {
public:
  database_file(const std::string &db_name, const std::string &init_sql, const std::string &db_dir = "./",
                const std::string &filename = "", long backup_freq = 0);
  ~database_file();
  bool open();
  void close();
  bool executeQuery(std::string sSqlInsert);
  int selectSumOrCount(std::string sSqlSelectCount);
  std::shared_ptr<database_select_rows> selectRows(std::string sqlSelectRows);

private:
  void copy_database_to_backup();
  std::mutex m_mutex;

  std::string TAG;
  void *m_database_file_db;
  std::string m_sFilename;
  std::string m_sFileFullpath;
  long m_backup_freq_in_seconds;
  std::string m_sBaseFileBackupFullpath;
  std::string m_init_sql;
  int m_nLastBackupTime;
};

} // namespace ctf01d