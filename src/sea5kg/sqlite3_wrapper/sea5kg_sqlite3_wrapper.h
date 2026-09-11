/**********************************************************************************
 * MIT License
 *
 * Copyright (c) 2025-2026 Evgenii Sopov <mrseakg@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 *all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Official Source Code: https://github.com/sea5kg/sea5kg-sql-builder
 *
 ***********************************************************************************/

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace sea5kg {

namespace sqlite3_wrapper {

class database_update_info {
public:
  database_update_info(const std::string &version_from, const std::string &version_to, const std::string &description);
  const std::string &version_from() const;
  const std::string &version_to() const;
  const std::string &description() const;

private:
  std::string m_version_from;
  std::string m_version_to;
  std::string m_description;
};

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

class database_update {
public:
  database_update(
    const std::string &db_name,
    const std::string &version_from,
    const std::string &version_to,
    const std::string &description
  );
  const database_update_info &info();
  const std::string &db_name() const;
  void set_weight(int weight);
  int weight();
  virtual bool apply_update(database_file *db, std::string &error) = 0;

protected:
  std::string TAG;

private:
  database_update_info m_update_info;
  std::string m_db_name;
  int m_weight;
};

#define CLASS_DATABASE_UPDATE_BEGIN(class_name, ver_from, ver_to, description) \
  class db_update_##class_name##_##ver_from##_##ver_to; \
  struct registry_db_update_fabric_##class_name##_##ver_from##_##ver_to { \
    registry_db_update_fabric_##class_name##_##ver_from##_##ver_to() { \
      std::shared_ptr<sea5kg::sqlite3_wrapper::database_update_fabric_base> ptr = std::make_shared< \
        sea5kg::sqlite3_wrapper::database_update_fabric<db_update_##class_name##_##ver_from##_##ver_to>>(); \
      sea5kg::sqlite3_wrapper::global::registry_database_update_fabric(#class_name, ptr); \
    } \
  } registry_db_update_fabric_##class_name##_##ver_from##_##ver_to##__; \
  class db_update_##class_name##_##ver_from##_##ver_to : public sea5kg::sqlite3_wrapper::database_update { \
  public: \
    db_update_##class_name##_##ver_from##_##ver_to() \
        : sea5kg::sqlite3_wrapper::database_update(#class_name, #ver_from, #ver_to, description) { \
    } \
    virtual bool apply_update(sea5kg::sqlite3_wrapper::database_file * db, std::string & error) override

#define CLASS_DATABASE_UPDATE_END() \
  } \
  ;

#define CLASS_DATABASE_UPDATE_NEXT(class_name, ver_from, ver_to, description) \
  CLASS_DATABASE_UPDATE_END() \
  CLASS_DATABASE_UPDATE_BEGIN(class_name, ver_from, ver_to, description)

class rows_iterator {
public:
  virtual bool next() = 0;
  virtual std::string as_string(int column_idx) = 0;
  virtual long as_long(int column_idx) = 0;
};

class database_file {
public:
  database_file(
    const std::string &db_name,
    const std::string &init_sql,
    const std::string &db_dir = "./",
    const std::string &filename = "",
    long backup_freq = 0
  );
  ~database_file();
  const std::string &filename() const;
  const std::string &filepath() const;
  bool open(std::string &error);
  bool is_opened() const;
  void close();
  bool contains_table(const std::string &table_name);
  bool execute_query(const std::string &sql, std::string &error);
  int select_sum_or_count(const std::string &sql, std::string &error);
  std::shared_ptr<rows_iterator> select_rows(const std::string &sql, std::string &error);
  bool copy_database_to_backup(std::string &error);

private:
  bool create_table_db_version(std::string &error);
  bool install_updates(std::string &error);
  bool insert_db_version(const database_update_info &info, std::string &error);

  std::mutex m_mutex;

  std::string TAG;
  std::string m_init_sql;
  void *m_db; // hidden type 'sqlite3 *'
  std::string m_db_name;
  std::string m_filename;
  std::string m_initial_version;
  std::string m_filepath;
  std::string m_basename_backup_filepath;
  std::string m_last_error;
  long m_last_backup_time;
  long m_backup_freq_in_seconds;
  std::vector<std::shared_ptr<database_update>> m_db_updates;
};

} // namespace sqlite3_wrapper

} // namespace sea5kg
