/**********************************************************************************
 *          Project
 *  _______ _________ _______  _______  __    ______
 * (  ____ \\__   __/(  ____ \(  __   )/  \  (  __  \
 * | (    \/   ) (   | (    \/| (  )  |\/) ) | (  \  )
 * | |         | |   | (__    | | /   |  | | | |   ) |
 * | |         | |   |  __)   | (/ /) |  | | | |   | |
 * | |         | |   | (      |   / | |  | | | |   ) |
 * | (____/\   | |   | )      |  (__) |__) (_| (__/  )
 * (_______/   )_(   |/       (_______)\____/(______/
 *
 * MIT License
 * Copyright (c) 2018-2025 Evgenii Sopov
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
 *
 ***********************************************************************************/

#pragma once

#include <string>
#include <map>
#include <mutex>
#include <memory>

class Ctf01dDatabaseFile;

extern std::map<std::string, Ctf01dDatabaseFile *> *g_pOpenedDatabaseFiles;

class Ctf01dDatabase {
public:
  static void addOpenedDatabaseFile(const std::string &name, Ctf01dDatabaseFile *db);
  static bool initDriverSqlite3(int &ret);
  static void shutdownDriverSqlite3();
};

class Ctf01dDatabaseSelectRows {
public:
  virtual bool next() = 0;
  virtual std::string getString(int nColumnNumber) = 0;
  virtual long getLong(int nColumnNumber) = 0;
};

class Ctf01dDatabaseFile {
public:
  Ctf01dDatabaseFile(const std::string &sFilename, const std::string &sSqlCreateTable);
  ~Ctf01dDatabaseFile();
  bool open();
  void close();
  bool executeQuery(std::string sSqlInsert);
  int selectSumOrCount(std::string sSqlSelectCount);
  std::shared_ptr<Ctf01dDatabaseSelectRows> selectRows(std::string sqlSelectRows);

private:

  void copyDatabaseToBackup();
  std::mutex m_mutex;

  std::string TAG;
  void *m_pDatabaseFile;
  std::string m_sFilename;
  std::string m_sFileFullpath;
  std::string m_sBaseFileBackupFullpath;
  std::string m_sSqlCreateTable;
  int m_nLastBackupTime;
};
