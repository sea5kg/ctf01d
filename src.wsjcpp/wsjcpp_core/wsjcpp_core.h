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

// original source-code: https://github.com/wsjcpp/wsjcpp-core

#ifndef WSJCPP_CORE_H
#define WSJCPP_CORE_H

#define WSJCPP_CORE_VER_0
#define WSJCPP_CORE_VER_0_3
#define WSJCPP_CORE_VER_0_3_0

#include <deque>
#include <iostream>
#include <map>
#include <mutex>
#include <regex>
#include <string>
#include <vector>

class WsjcppFilePermissions {
public:
  WsjcppFilePermissions();
  WsjcppFilePermissions(bool bOwnerReadFlag, bool bOwnerWriteFlag, bool bOwnerExecuteFlag, bool bGroupReadFlag,
                        bool bGroupWriteFlag, bool bGroupExecuteFlag, bool bOtherReadFlag, bool bOtherWriteFlag,
                        bool bOtherExecuteFlag);
  WsjcppFilePermissions(uint16_t nFilePermission);

  // owner flags
  void setOwnerReadFlag(bool bOwnerReadFlag);
  bool getOwnerReadFlag() const;
  void setOwnerWriteFlag(bool bOwnerWriteFlag);
  bool getOwnerWriteFlag() const;
  void setOwnerExecuteFlag(bool bOwnerExecuteFlag);
  bool getOwnerExecuteFlag() const;
  void setOwnerFlags(bool bOwnerReadFlag, bool bOwnerWriteFlag, bool bOwnerExecuteFlag);

  // group flags
  void setGroupReadFlag(bool bGroupReadFlag);
  bool getGroupReadFlag() const;
  void setGroupWriteFlag(bool bGroupWriteFlag);
  bool getGroupWriteFlag() const;
  void setGroupExecuteFlag(bool bGroupExecuteFlag);
  bool getGroupExecuteFlag() const;
  void setGroupFlags(bool bGroupReadFlag, bool bGroupWriteFlag, bool bGroupExecuteFlag);

  // for other flags
  void setOtherReadFlag(bool bOtherReadFlag);
  bool getOtherReadFlag() const;
  void setOtherWriteFlag(bool bOtherWriteFlag);
  bool getOtherWriteFlag() const;
  void setOtherExecuteFlag(bool bOtherExecuteFlag);
  bool getOtherExecuteFlag() const;
  void setOtherFlags(bool bOtherReadFlag, bool bOtherWriteFlag, bool bOtherExecuteFlag);

  std::string toString() const;
  uint16_t toUInt16() const;

private:
  bool m_bOwnerReadFlag;
  bool m_bOwnerWriteFlag;
  bool m_bOwnerExecuteFlag;
  bool m_bGroupReadFlag;
  bool m_bGroupWriteFlag;
  bool m_bGroupExecuteFlag;
  bool m_bOtherReadFlag;
  bool m_bOtherWriteFlag;
  bool m_bOtherExecuteFlag;
};

class WsjcppCore {
public:
  static bool init(int argc, char **argv, const std::string &sApplicationName, const std::string &sApplicationVersion,
                   const std::string &sApplicationAuthor, const std::string &sLibraryNameForExports);

  static std::string extractDirpath(const std::string &sFullPath);
  static std::string getCurrentDirectory();

  static long getCurrentTimeInMilliseconds();
  static long getCurrentTimeInSeconds();
  static std::string getCurrentTimeForFilename();
  static std::string getCurrentTimeForLogFormat();
  static std::string getThreadId();
  static std::string formatTimeForWeb(long nTimeInSec);
  static std::string formatTimeForFilename(long nTimeInSec);
  static std::string formatTimeUTC(int nTimeInSec);

  static std::vector<std::string> getListOfFiles(const std::string &sDirname);
  static bool makeDir(const std::string &sDirname);
  static bool makeDirsPath(const std::string &sDirname);
  static bool writeFile(const std::string &sFilename, const std::string &sContent);
  static bool readTextFile(const std::string &sFilename, std::string &sOutputContent);
  static bool readFileToBuffer(const std::string &sFilename, char *pBuffer[], int &nBufferSize);
  static bool writeFile(const std::string &sFilename, const char *pBuffer, const int nBufferSize);
  static bool removeFile(const std::string &sFilename);
  static bool copyFile(const std::string &sSourceFilename, const std::string &sTargetFilename);

  static std::string &ltrim(std::string &str, const std::string &chars = "\t\n\v\f\r ");
  static std::string &rtrim(std::string &str, const std::string &chars = "\t\n\v\f\r ");
  static std::string &trim(std::string &str, const std::string &chars = "\t\n\v\f\r ");
  static std::string toLower(const std::string &str);
  static std::string toUpper(const std::string &str);

  static void initRandom();
  static std::string uint2hexString(unsigned int n);
  static unsigned long convertVoidToULong(void *p);
  static std::string getPointerAsHex(void *p);
  static bool getEnv(const std::string &sName, std::string &sValue);

  static std::string encodeUriComponent(const std::string &sValue);
  static std::string decodeUriComponent(const std::string &sValue);

  static std::string getHumanSizeBytes(long nBytes);

  static bool setFilePermissions(const std::string &sFilePath, const WsjcppFilePermissions &filePermissions,
                                 std::string &sError);
  static bool getFilePermissions(const std::string &sFilePath, WsjcppFilePermissions &filePermissions,
                                 std::string &sError);
};

enum WsjcppColorCode { // deprecated, see https://github.com/sea5kg/sea5kg-logger
  FG_RED = 31,
  FG_GREEN = 32,
  FG_YELLOW = 93,
  FG_BLUE = 34,
  FG_DEFAULT = 39,
  BG_RED = 41,
  BG_GREEN = 42,
  BG_BLUE = 44,
  BG_DEFAULT = 49
};

class WsjcppColorModifier { // deprecated, see https://github.com/sea5kg/sea5kg-logger
  WsjcppColorCode code;

public:
  WsjcppColorModifier(WsjcppColorCode pCode) : code(pCode) {
  }
  friend std::ostream &operator<<(std::ostream &os, const WsjcppColorModifier &mod) {
    return os << "\033[" << mod.code << "m";
  }
};

// ---------------------------------------------------------------------

class WsjcppLogGlobalConf { // deprecated, see https://github.com/sea5kg/sea5kg-logger
public:
  WsjcppLogGlobalConf();
  void doLogRotateUpdateFilename(bool bForce = false);
  std::mutex logMutex;
  std::string logDir;
  std::string logPrefixFile;
  std::string logFile;
  bool enableLogFile;
  long logStartTime;
  long logRotationPeriodInSeconds;
  std::deque<std::string> logLastMessages;
};

class WsjcppLog { // deprecated, see https://github.com/sea5kg/sea5kg-logger
public:
  static WsjcppLogGlobalConf g_WSJCPP_LOG_GLOBAL_CONF;

  static void info(const std::string &sTag, const std::string &sMessage);
  static void err(const std::string &sTag, const std::string &sMessage);
  static void throw_err(const std::string &sTag, const std::string &sMessage);
  static void warn(const std::string &sTag, const std::string &sMessage);
  static void ok(const std::string &sTag, const std::string &sMessage);
  static std::vector<std::string> getLastLogMessages();
  static void setLogDirectory(const std::string &sDirectoryPath);
  static void setPrefixLogFile(const std::string &sPrefixLogFile);
  static void setEnableLogFile(bool bEnable);
  static void setRotationPeriodInSec(long nRotationPeriodInSec);

private:
  static void add(WsjcppColorModifier &clr, const std::string &sType, const std::string &sTag,
                  const std::string &sMessage);
};

// ---------------------------------------------------------------------
// WsjcppResourceFile

class WsjcppResourceFile {
public:
  WsjcppResourceFile();
  virtual const std::string &getFilename() const = 0;
  virtual const std::string &getPackAs() const = 0;
  virtual int getBufferSize() const = 0;
  virtual const char *getBuffer() const = 0;
};

// ---------------------------------------------------------------------
// WsjcppResourcesManager

extern std::vector<WsjcppResourceFile *> *g_pWsjcppResourceFiles;

class WsjcppResourcesManager {
public:
  static void initGlobalVariables();
  static void add(WsjcppResourceFile *);
  static const std::vector<WsjcppResourceFile *> &list();
  static bool has(const std::string &sFilename);
  static WsjcppResourceFile *get(const std::string &sFilename);
  static bool make(const std::string &sWorkspace);
  // static bool createFolders(const std::string &sWorkspace);
  // static bool extractFiles(const std::string &sWorkspace);
};

// ---------------------------------------------------------------------
// Registry WsjcppResourceFile
#define REGISTRY_WSJCPP_RESOURCE_FILE(classname)                                                                       \
  static classname *pRegistryWsjcppResourceFile##classname = new classname();

namespace wsjcpp {

class Core {
public:
  static const std::string &englishAlphabetLowerCase();
  static const std::string &englishAlphabetUpperCase();
  static const std::string &englishAlphabetBothCase();
  static const std::string &englishAlphabetBothCaseAndNumbers();
  static std::string randomString(const std::string &alphabet, int length);
};

std::string normalize_filepath(const std::string &path);
std::string extract_url_protocol(const std::string &sValue);
bool recursive_copy_files(const std::string &source_dir, const std::string &target_dir, std::string &error);
bool recursive_remove_dir(const std::string &dirpath, std::string &error);
bool starts_with(const std::string &str, const std::string &start_str);
bool ends_with(const std::string &str, const std::string &end_str);
std::vector<std::string> directory_list(const std::string &dirpath);
std::string generate_uuid();
std::string parent_dirpath(const std::string &filepath);
std::string to_snake_case(const std::string &name);
bool dir_exists(const std::string &dir_path);
bool file_exists(const std::string &file_path);
std::vector<std::string> split(const std::string &source, const std::string &delimiter);
std::string join(const std::vector<std::string> &source, const std::string &delimiter);
void replace_all_in(std::string &target, const std::string &from, const std::string &to);
bool create_empty_file(const std::string &filepath);
std::string extract_filename(const std::string &filepath);
int levenshtein_distance(const std::string &str1, const std::string &str2);
bool user_is_root();
bool change_current_process_privileges(int user_id, std::string &error);
enum class operation_system { UNKNOWN, LINUX, WINDOWS };
operation_system operation_system_platform();
bool is_linux();
bool is_windows();
std::string padding_right(const std::string &source, char pad_char, size_t length);
std::string padding_left(const std::string &source, char pad_char, size_t length);
bool is_valid_ip4(const std::string &value, std::string &error);
bool is_valid_ip6(const std::string &value, std::string &error);
bool is_valid_date(const std::string &value, std::string &error);
bool is_valid_time24(const std::string &value, std::string &error);
bool is_valid_url_protocol(const std::string &value, std::string &error);
bool is_valid_domain_name(const std::string &value, std::string &error);
bool is_valid_port(const std::string &value, std::string &error);
bool is_valid_port(int nValue, std::string &error);
bool is_valid_base64(const std::string &value, std::string &error);

// helpful validators

template <typename T> class validator {
public:
  validator(const std::string &name) : m_name(name) {
  }
  virtual const std::string &name() const {
    return m_name;
  };
  virtual bool is_valid(const T &value, std::string &error) = 0;

private:
  std::string m_name;
};

class validator_date : public validator<std::string> {
public:
  validator_date();
  virtual bool is_valid(const std::string &value, std::string &error) override;
};

class validator_regexp : public validator<std::string> {
public:
  validator_regexp(const std::string &typeName, const std::string &sValidator);
  virtual bool is_valid(const std::string &value, std::string &error) override;

private:
  std::string m_sValidator;
  std::regex m_rxValidator;
};

class validator_str_list : public validator<std::string> {
public:
  validator_str_list(const std::string &name, const std::vector<std::string> &list_vals);
  virtual bool is_valid(const std::string &value, std::string &error) override;

private:
  std::vector<std::string> m_vListValues;
};

class validator_email : public validator_regexp {
public:
  validator_email();
};

class validator_uuid : public validator_regexp {
public:
  validator_uuid();
};

class validator_strlen : public validator<std::string> {
public:
  validator_strlen(int min_length, int max_length);
  virtual bool is_valid(const std::string &value, std::string &error) override;

private:
  int m_min_length;
  int m_max_length;
};

class validator_jwt : public validator_regexp {
public:
  validator_jwt();
};

class validator_time24 : public validator<std::string> {
public:
  validator_time24();
  virtual bool is_valid(const std::string &value, std::string &error) override;
};

class validator_datetime : public validator<std::string> {
public:
  validator_datetime();
  virtual bool is_valid(const std::string &value, std::string &error) override;
};

class validator_url : public validator<std::string> {
public:
  validator_url();
  virtual bool is_valid(const std::string &value, std::string &error) override;

private:
  std::regex m_rx_ip4_format;
};

class validator_domain_name : public validator<std::string> {
public:
  validator_domain_name();
  virtual bool is_valid(const std::string &value, std::string &error) override;
};

class validator_base64 : public validator<std::string> {
public:
  validator_base64();
  virtual bool is_valid(const std::string &value, std::string &error) override;
};

class validator_int : public validator<std::string> {
public:
  validator_int();
  virtual bool is_valid(const std::string &value, std::string &error) override;
};

class validator_hex : public validator<std::string> {
public:
  validator_hex();
  virtual bool is_valid(const std::string &value, std::string &error) override;
};

class validator_int_min : public validator<int> {
public:
  validator_int_min(int min_value);
  virtual bool is_valid(const int &value, std::string &error) override;

private:
  int m_min_value;
};

class validator_int_max : public validator<int> {
public:
  validator_int_max(int max_value);
  virtual bool is_valid(const int &value, std::string &error) override;

private:
  int m_max_value;
};

} // namespace wsjcpp

#endif // WSJCPP_CORE_H
