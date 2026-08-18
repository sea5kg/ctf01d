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

#include "wsjcpp_core.h"

#ifndef _MSC_VER
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/time.h>
#include <unistd.h>
#else
#include <direct.h>
#define PATH_MAX 256
#endif

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>
#include <streambuf>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <time.h>
// #include <sys/socket.h>
#include <iomanip>
#include <random>

// ---------------------------------------------------------------------
// WsjcppFilePermissions

WsjcppFilePermissions::WsjcppFilePermissions() {
  // default permissions
  m_bOwnerReadFlag = true;
  m_bOwnerWriteFlag = true;
  m_bOwnerExecuteFlag = false;
  m_bGroupReadFlag = false;
  m_bGroupWriteFlag = false;
  m_bGroupExecuteFlag = false;
  m_bOtherReadFlag = true;
  m_bOtherWriteFlag = false;
  m_bOtherExecuteFlag = false;
}

WsjcppFilePermissions::WsjcppFilePermissions(bool bOwnerReadFlag, bool bOwnerWriteFlag, bool bOwnerExecuteFlag,
                                             bool bGroupReadFlag, bool bGroupWriteFlag, bool bGroupExecuteFlag,
                                             bool bOtherReadFlag, bool bOtherWriteFlag, bool bOtherExecuteFlag) {
  m_bOwnerReadFlag = bOwnerReadFlag;
  m_bOwnerWriteFlag = bOwnerWriteFlag;
  m_bOwnerExecuteFlag = bOwnerExecuteFlag;
  m_bGroupReadFlag = bGroupReadFlag;
  m_bGroupWriteFlag = bGroupWriteFlag;
  m_bGroupExecuteFlag = bGroupExecuteFlag;
  m_bOtherReadFlag = bOtherReadFlag;
  m_bOtherWriteFlag = bOtherWriteFlag;
  m_bOtherExecuteFlag = bOtherExecuteFlag;
}

WsjcppFilePermissions::WsjcppFilePermissions(uint16_t nFilePermission) {

  // owner
  m_bOwnerReadFlag = nFilePermission & 0x0400;
  m_bOwnerWriteFlag = nFilePermission & 0x0200;
  m_bOwnerExecuteFlag = nFilePermission & 0x0100;

  // group
  m_bGroupReadFlag = nFilePermission & 0x0040;
  m_bGroupWriteFlag = nFilePermission & 0x0020;
  m_bGroupExecuteFlag = nFilePermission & 0x0010;

  // for other
  m_bOtherReadFlag = nFilePermission & 0x0004;
  m_bOtherWriteFlag = nFilePermission & 0x0002;
  m_bOtherExecuteFlag = nFilePermission & 0x0001;
}

void WsjcppFilePermissions::setOwnerReadFlag(bool bOwnerReadFlag) {
  m_bOwnerReadFlag = bOwnerReadFlag;
}

bool WsjcppFilePermissions::getOwnerReadFlag() const {
  return m_bOwnerReadFlag;
}

void WsjcppFilePermissions::setOwnerWriteFlag(bool bOwnerWriteFlag) {
  m_bOwnerWriteFlag = bOwnerWriteFlag;
}

bool WsjcppFilePermissions::getOwnerWriteFlag() const {
  return m_bOwnerWriteFlag;
}

void WsjcppFilePermissions::setOwnerExecuteFlag(bool bOwnerExecuteFlag) {
  m_bOwnerExecuteFlag = bOwnerExecuteFlag;
}

bool WsjcppFilePermissions::getOwnerExecuteFlag() const {
  return m_bOwnerExecuteFlag;
}

void WsjcppFilePermissions::setOwnerFlags(bool bOwnerReadFlag, bool bOwnerWriteFlag, bool bOwnerExecuteFlag) {
  m_bOwnerReadFlag = bOwnerReadFlag;
  m_bOwnerWriteFlag = bOwnerWriteFlag;
  m_bOwnerExecuteFlag = bOwnerExecuteFlag;
}

void WsjcppFilePermissions::setGroupReadFlag(bool bGroupReadFlag) {
  m_bGroupReadFlag = bGroupReadFlag;
}

bool WsjcppFilePermissions::getGroupReadFlag() const {
  return m_bGroupReadFlag;
}

void WsjcppFilePermissions::setGroupWriteFlag(bool bGroupWriteFlag) {
  m_bGroupWriteFlag = bGroupWriteFlag;
}

bool WsjcppFilePermissions::getGroupWriteFlag() const {
  return m_bGroupWriteFlag;
}

void WsjcppFilePermissions::setGroupExecuteFlag(bool bGroupExecuteFlag) {
  m_bGroupExecuteFlag = bGroupExecuteFlag;
}

bool WsjcppFilePermissions::getGroupExecuteFlag() const {
  return m_bGroupExecuteFlag;
}

void WsjcppFilePermissions::setGroupFlags(bool bGroupReadFlag, bool bGroupWriteFlag, bool bGroupExecuteFlag) {
  m_bGroupReadFlag = bGroupReadFlag;
  m_bGroupWriteFlag = bGroupWriteFlag;
  m_bGroupExecuteFlag = bGroupExecuteFlag;
}

void WsjcppFilePermissions::setOtherReadFlag(bool bOtherReadFlag) {
  m_bOtherReadFlag = bOtherReadFlag;
}

bool WsjcppFilePermissions::getOtherReadFlag() const {
  return m_bOtherReadFlag;
}

void WsjcppFilePermissions::setOtherWriteFlag(bool bOtherWriteFlag) {
  m_bOtherWriteFlag = bOtherWriteFlag;
}

bool WsjcppFilePermissions::getOtherWriteFlag() const {
  return m_bOtherWriteFlag;
}

void WsjcppFilePermissions::setOtherExecuteFlag(bool bOtherExecuteFlag) {
  m_bOtherExecuteFlag = bOtherExecuteFlag;
}

bool WsjcppFilePermissions::getOtherExecuteFlag() const {
  return m_bOtherExecuteFlag;
}

void WsjcppFilePermissions::setOtherFlags(bool bOtherReadFlag, bool bOtherWriteFlag, bool bOtherExecuteFlag) {
  m_bOtherReadFlag = bOtherReadFlag;
  m_bOtherWriteFlag = bOtherWriteFlag;
  m_bOtherExecuteFlag = bOtherExecuteFlag;
}

std::string WsjcppFilePermissions::toString() const {
  std::string sRet = "-";

  // owner
  sRet += m_bOwnerReadFlag ? "r" : "-";
  sRet += m_bOwnerWriteFlag ? "w" : "-";
  sRet += m_bOwnerExecuteFlag ? "x" : "-";

  // group
  sRet += m_bGroupReadFlag ? "r" : "-";
  sRet += m_bGroupWriteFlag ? "w" : "-";
  sRet += m_bGroupExecuteFlag ? "x" : "-";

  // for other
  sRet += m_bOtherReadFlag ? "r" : "-";
  sRet += m_bOtherWriteFlag ? "w" : "-";
  sRet += m_bOtherExecuteFlag ? "x" : "-";

  return sRet;
}

uint16_t WsjcppFilePermissions::toUInt16() const {
  uint16_t nRet = 0x0;

  // owner
  nRet |= m_bOwnerReadFlag ? 0x0400 : 0x0;
  nRet |= m_bOwnerWriteFlag ? 0x0200 : 0x0;
  nRet |= m_bOwnerExecuteFlag ? 0x0100 : 0x0;

  // group
  nRet += m_bGroupReadFlag ? 0x0040 : 0x0;
  nRet += m_bGroupWriteFlag ? 0x0020 : 0x0;
  nRet += m_bGroupExecuteFlag ? 0x0010 : 0x0;

  // for other
  nRet += m_bOtherReadFlag ? 0x0004 : 0x0;
  nRet += m_bOtherWriteFlag ? 0x0002 : 0x0;
  nRet += m_bOtherExecuteFlag ? 0x0001 : 0x0;
  return nRet;
}

// ---------------------------------------------------------------------
// WsjcppCore

bool WsjcppCore::init(int argc, char **argv, const std::string &sApplicationName,
                      const std::string &sApplicationVersion, const std::string &sApplicationAuthor,
                      const std::string &sLibraryNameForExports) {
  // init random
  // std::srand(std::time(0));
  WsjcppCore::initRandom();
  return true;
}

std::string WsjcppCore::extractDirpath(const std::string &sFullPath) {
  std::vector<std::string> vDirs = wsjcpp::split(sFullPath, "/");
  vDirs.pop_back();
  return wsjcpp::join(vDirs, "/");
}

std::string WsjcppCore::getCurrentDirectory() {
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    WsjcppLog::throw_err("getCurrentDirectory", "Could not get current directory");
  }
  return std::string(cwd) + "/";
}

long WsjcppCore::getCurrentTimeInMilliseconds() {
  long nTimeStart =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
          .count();
  return nTimeStart;
}

long WsjcppCore::getCurrentTimeInSeconds() {
  long nTimeStart =
      std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  return nTimeStart;
}

std::string WsjcppCore::getCurrentTimeForFilename() {
  long nTimeStart = WsjcppCore::getCurrentTimeInSeconds();
  return WsjcppCore::formatTimeForFilename(nTimeStart);
}

std::string WsjcppCore::getCurrentTimeForLogFormat() {
  long nTimeStart = WsjcppCore::getCurrentTimeInMilliseconds();
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

std::string WsjcppCore::getThreadId() {
  static_assert(sizeof(std::thread::id) == sizeof(uint64_t),
                "this function only works if size of thead::id is equal to the size of uint_64");
  std::thread::id this_id = std::this_thread::get_id();
  uint64_t val = *((uint64_t *)&this_id);
  std::stringstream stream;
  stream << "0x" << std::setw(16) << std::setfill('0') << std::hex << val;
  return std::string(stream.str());
}

std::string WsjcppCore::formatTimeForWeb(long nTimeInSec) {
  std::time_t tm_ = long(nTimeInSec);
  // struct tm tstruct = *localtime(&tm_);
  struct tm tstruct;
  gmtime_r(&tm_, &tstruct);

  // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
  // for more information about date/time format
  char buf[80];
  // Like: Wed, 22 Jul 2009 19:15:56 GMT
  strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tstruct);
  return std::string(buf);
}

std::string WsjcppCore::formatTimeForFilename(long nTimeInSec) {
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

std::string WsjcppCore::formatTimeUTC(int nTimeInSec) {
  // datetime
  std::time_t tm_ = long(nTimeInSec);
  // struct tm tstruct = *localtime(&tm_);
  struct tm tstruct;
  gmtime_r(&tm_, &tstruct);

  // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
  // for more information about date/time format
  char buf[80];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
  return std::string(buf);
}

std::vector<std::string> WsjcppCore::getListOfFiles(const std::string &sDirname) {
  std::vector<std::string> vFiles;
  if (!wsjcpp::dir_exists(sDirname)) {
    return vFiles;
  }
  for (auto &entry : std::filesystem::directory_iterator(sDirname)) {
    if (!entry.is_directory()) {
      std::string sPath = entry.path();
      sPath.erase(0, sDirname.size() + 1);
      vFiles.push_back(sPath);
    }
  }
  return vFiles;
}

bool WsjcppCore::makeDir(const std::string &sDirname) {
  struct stat st;

  const std::filesystem::path dir{sDirname};
  std::filesystem::create_directory(dir);

  int nStatus = mkdir(sDirname.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if (nStatus == 0) {
    return true;
  }
  if (nStatus == EACCES) {
    std::cout << "FAILED create folder " << sDirname << std::endl;
    return false;
  }
  // std::cout << "nStatus: " << nStatus << std::endl;
  return true;
}

bool WsjcppCore::makeDirsPath(const std::string &dirpath) {
  std::string sDirpath = wsjcpp::normalize_filepath(dirpath);
  std::vector<std::string> vDirs = wsjcpp::split(sDirpath, "/");
  std::string sDirpath2 = "";
  if (sDirpath.length() > 0 && sDirpath[0] == '/') {
    sDirpath2 = "/";
  }
  for (int i = 0; i < vDirs.size(); i++) {
    if (vDirs[i] == "") {
      continue;
    }
    sDirpath2 += vDirs[i] + "/";
    if (wsjcpp::dir_exists(sDirpath2)) {
      continue;
    } else {
      if (!WsjcppCore::makeDir(sDirpath2)) {
        return false;
      }
    }
  }
  return true;
}

bool WsjcppCore::writeFile(const std::string &sFilename, const std::string &sContent) {
  // std::ofstream f(sFilename, std::ifstream::in);
  std::ofstream f(sFilename, std::ios::out);
  if (!f) {
    WsjcppLog::err("WsjcppCore", "Could not create file to write '" + sFilename + "'");
    return false;
  }
  f << sContent << std::endl;
  f.close();
  return true;
}

bool WsjcppCore::readTextFile(const std::string &sFilename, std::string &sContent) {
  std::ifstream f(sFilename);
  if (!f) {
    std::cout << "FAILED could not open file to read " << sFilename << std::endl;
    return false;
  }
  sContent = std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  return true;
}

bool WsjcppCore::readFileToBuffer(const std::string &sFilename, char *pBuffer[], int &nBufferSize) {
  std::ifstream f(sFilename, std::ifstream::binary);
  if (!f) {
    return false;
  }

  // get length of file:
  f.seekg(0, f.end);
  nBufferSize = f.tellg();
  f.seekg(0, f.beg);

  *pBuffer = new char[nBufferSize];

  // read data as a block:
  f.read(*pBuffer, nBufferSize);
  if (!f) {
    WsjcppLog::err("WsjcppCore::readFileToBuffer", "Only " + std::to_string(f.gcount()) + " could be read");
    delete[] pBuffer;
    f.close();
    return false;
  }
  f.close();
  return true;
}

bool WsjcppCore::writeFile(const std::string &sFilename, const char *pBuffer, const int nBufferSize) {
  std::ofstream f(sFilename, std::ios::out | std::ios::binary);
  if (!f) {
    std::cout << "FAILED could not create file to write " << sFilename << std::endl;
    return false;
  }
  f.write(pBuffer, nBufferSize);
  f.close();
  return true;
}

bool WsjcppCore::removeFile(const std::string &sFilename) {
  return remove(sFilename.c_str()) == 0;
}

bool WsjcppCore::copyFile(const std::string &sSourceFilename, const std::string &sTargetFilename) {
  if (!wsjcpp::file_exists(sSourceFilename)) {
    WsjcppLog::err("copyFile", "File '" + sSourceFilename + "' did not exists");
    return false;
  }
  if (wsjcpp::file_exists(sTargetFilename)) {
    WsjcppLog::err("copyFile", "File '" + sTargetFilename + "' already exists");
    return false;
  }
  std::ifstream src(sSourceFilename, std::ios::binary);
  if (!src.is_open()) {
    WsjcppLog::err("copyFile", "Could not open file '" + sSourceFilename + "' for read");
    return false;
  }
  std::ofstream dst(sTargetFilename, std::ios::binary);
  if (!dst.is_open()) {
    WsjcppLog::err("copyFile", "Could not open file '" + sTargetFilename + "' for write");
    return false;
  }
  dst << src.rdbuf();
  return true;
}

std::string &WsjcppCore::ltrim(std::string &str, const std::string &chars) {
  str.erase(0, str.find_first_not_of(chars));
  return str;
}

std::string &WsjcppCore::rtrim(std::string &str, const std::string &chars) {
  str.erase(str.find_last_not_of(chars) + 1);
  return str;
}

std::string &WsjcppCore::trim(std::string &str, const std::string &chars) {
  return WsjcppCore::ltrim(WsjcppCore::rtrim(str, chars), chars);
}

// ---------------------------------------------------------------------
// will worked only with latin

std::string WsjcppCore::toLower(const std::string &str) {
  std::string sRet = str;
  std::transform(sRet.begin(), sRet.end(), sRet.begin(), ::tolower);
  return sRet;
}

// ---------------------------------------------------------------------
// will worked only with latin

std::string WsjcppCore::toUpper(const std::string &str) {
  std::string sRet = str;
  std::transform(sRet.begin(), sRet.end(), sRet.begin(), ::toupper);
  return sRet;
}

void WsjcppCore::initRandom() {
  time_t t = std::time(0);
  std::srand((unsigned int)t);
}

std::string WsjcppCore::uint2hexString(unsigned int n) {
  std::string sRet;
  for (int i = 0; i < 8; i++) {
    sRet += "0123456789abcdef"[n % 16];
    n >>= 4;
  }
  return std::string(sRet.rbegin(), sRet.rend());
}

unsigned long WsjcppCore::convertVoidToULong(void *p) {
  std::uintptr_t ret = reinterpret_cast<std::uintptr_t>(p);
  return (unsigned long)ret;
}

std::string WsjcppCore::getPointerAsHex(void *p) {
  std::uintptr_t i = reinterpret_cast<std::uintptr_t>(p);
  std::stringstream stream;
  stream << std::hex << i;
  return "0x" + std::string(stream.str());
}

bool WsjcppCore::getEnv(const std::string &sName, std::string &sValue) {
  if (const char *env_p = std::getenv(sName.c_str())) {
    sValue = std::string(env_p);
    return true;
  }
  return false;
}

std::string WsjcppCore::encodeUriComponent(const std::string &sValue) {
  std::stringstream ssRet;
  for (int i = 0; i < sValue.length(); i++) {
    char c = sValue[i];
    if (c == '-' || c == '_' || c == '.' || c == '!' || c == '~' || c == '*' || c == '\'' || c == '(' || c == ')' ||
        (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
      ssRet << c;
    } else {
      ssRet << '%' << std::uppercase << std::hex << (int)c;
    }
  }
  return ssRet.str();
}

std::string WsjcppCore::decodeUriComponent(const std::string &sValue) {
  std::string sRet = "";
  std::string sHex = "";
  int nLen = sValue.length();
  for (int i = 0; i < sValue.length(); i++) {
    char c = sValue[i];
    if (c == '%') {
      if (i + 2 >= nLen) {
        WsjcppLog::throw_err("WsjcppCore::decodeUriElement", "Wrong format of string");
      }
      sHex = "0x";
      sHex += sValue[i + 1];
      sHex += sValue[i + 2];
      i = i + 2;
      char c1 = std::stoul(sHex, nullptr, 16);
      sRet += c1;
    } else {
      sRet += c;
    }
  }
  return sRet;
}

std::string WsjcppCore::getHumanSizeBytes(long nBytes) {
  if (nBytes == 0) {
    return "0B";
  }
  std::string arrPrefix[] = {"B", "KB", "MB", "GB", "TB", "PB"};
  long n0 = nBytes;
  long n1 = 0;
  for (int i = 0; i < 6; i++) {
    if (n0 >= 1 && n0 < 1000) {
      return std::to_string(n0) + arrPrefix[i];
    }
    n0 = nBytes / 1000;
    n1 = nBytes - n0 * 1000;
    n0 += n1 >= 500 ? 1 : 0;

    nBytes = nBytes / 1000;
    if (n0 == 0 && n1 == 0) {
      return "fuck";
    }
  }
  return std::to_string(nBytes) + "PB";
}

bool WsjcppCore::setFilePermissions(const std::string &sFilePath, const WsjcppFilePermissions &filePermissions,
                                    std::string &sError) {

  mode_t m = 0x0;

  // owner
  m |= filePermissions.getOwnerReadFlag() ? S_IRUSR : 0x0;
  m |= filePermissions.getOwnerWriteFlag() ? S_IWUSR : 0x0;
  m |= filePermissions.getOwnerExecuteFlag() ? S_IXUSR : 0x0;

  // group
  m |= filePermissions.getGroupReadFlag() ? S_IRGRP : 0x0;
  m |= filePermissions.getGroupWriteFlag() ? S_IWGRP : 0x0;
  m |= filePermissions.getGroupExecuteFlag() ? S_IXGRP : 0x0;

  // for other
  m |= filePermissions.getOtherReadFlag() ? S_IROTH : 0x0;
  m |= filePermissions.getOtherWriteFlag() ? S_IWOTH : 0x0;
  m |= filePermissions.getOtherExecuteFlag() ? S_IXOTH : 0x0;

  if (chmod(sFilePath.c_str(), m) != 0) {
    sError = "Could not change permissions for: '" + sFilePath + "'";
    return false;
  }
  return true;
}

bool WsjcppCore::getFilePermissions(const std::string &sFilePath, WsjcppFilePermissions &filePermissions,
                                    std::string &sError) {
  if (!wsjcpp::file_exists(sFilePath)) {
    sError = "File '" + sFilePath + "' - not found";
    return false;
  }

  struct stat fileStat;
  if (stat(sFilePath.c_str(), &fileStat) < 0) {
    sError = "Could not get info about file '" + sFilePath + "'.";
    return false;
  }

  mode_t m = fileStat.st_mode;

  // S_ISDIR(fileStat.st_mode)) ? "d" : "-"

  // owner
  filePermissions.setOwnerReadFlag(m & S_IRUSR);
  filePermissions.setOwnerWriteFlag(m & S_IWUSR);
  filePermissions.setOwnerExecuteFlag(m & S_IXUSR);

  // group
  filePermissions.setGroupReadFlag(m & S_IRGRP);
  filePermissions.setGroupWriteFlag(m & S_IWGRP);
  filePermissions.setGroupExecuteFlag(m & S_IXGRP);

  // for other
  filePermissions.setOtherReadFlag(m & S_IROTH);
  filePermissions.setOtherWriteFlag(m & S_IWOTH);
  filePermissions.setOtherExecuteFlag(m & S_IXOTH);

  return true;
}

// ---------------------------------------------------------------------
// WsjcppLog

WsjcppLogGlobalConf::WsjcppLogGlobalConf() {
  logDir = "./";
  logPrefixFile = "";
  logFile = "";
  enableLogFile = true;
  logStartTime = 0;
  logRotationPeriodInSeconds = 51000;
}

void WsjcppLogGlobalConf::doLogRotateUpdateFilename(bool bForce) {
  long t = WsjcppCore::getCurrentTimeInSeconds();
  long nEverySeconds = logRotationPeriodInSeconds; // rotate log if started now or if time left more then 1 day
  if (logStartTime == 0 || t - logStartTime > nEverySeconds || bForce) {
    logStartTime = t;
    logFile = logDir + "/" + logPrefixFile + "_" + WsjcppCore::formatTimeForFilename(logStartTime) + ".log";
  }
}

WsjcppLogGlobalConf WsjcppLog::g_WSJCPP_LOG_GLOBAL_CONF;

void WsjcppLog::info(const std::string &sTag, const std::string &sMessage) {
  WsjcppColorModifier def(WsjcppColorCode::FG_DEFAULT);
  WsjcppLog::add(def, "INFO", sTag, sMessage);
}

void WsjcppLog::err(const std::string &sTag, const std::string &sMessage) {
  WsjcppColorModifier red(WsjcppColorCode::FG_RED);
  WsjcppLog::add(red, "ERR", sTag, sMessage);
}

void WsjcppLog::throw_err(const std::string &sTag, const std::string &sMessage) {
  WsjcppColorModifier red(WsjcppColorCode::FG_RED);
  WsjcppLog::add(red, "ERR", sTag, sMessage);
  throw std::runtime_error(sMessage);
}

void WsjcppLog::warn(const std::string &sTag, const std::string &sMessage) {
  WsjcppColorModifier yellow(WsjcppColorCode::FG_YELLOW);
  WsjcppLog::add(yellow, "WARN", sTag, sMessage);
}

void WsjcppLog::ok(const std::string &sTag, const std::string &sMessage) {
  WsjcppColorModifier green(WsjcppColorCode::FG_GREEN);
  WsjcppLog::add(green, "OK", sTag, sMessage);
}

std::vector<std::string> WsjcppLog::getLastLogMessages() {
  std::lock_guard<std::mutex> lock(WsjcppLog::g_WSJCPP_LOG_GLOBAL_CONF.logMutex);
  std::vector<std::string> vRet;
  for (int i = 0; i < WsjcppLog::g_WSJCPP_LOG_GLOBAL_CONF.logLastMessages.size(); i++) {
    vRet.push_back(WsjcppLog::g_WSJCPP_LOG_GLOBAL_CONF.logLastMessages[i]);
  }
  return vRet;
}

void WsjcppLog::setLogDirectory(const std::string &sDirectoryPath) {
  WsjcppLog::g_WSJCPP_LOG_GLOBAL_CONF.logDir = sDirectoryPath;
  if (!wsjcpp::dir_exists(sDirectoryPath)) {
    if (!WsjcppCore::makeDir(sDirectoryPath)) {
      WsjcppLog::err("setLogDirectory", "Could not create log directory '" + sDirectoryPath + "'");
    }
  }
  WsjcppLog::g_WSJCPP_LOG_GLOBAL_CONF.doLogRotateUpdateFilename(true);
}

void WsjcppLog::setPrefixLogFile(const std::string &sPrefixLogFile) {
  WsjcppLog::g_WSJCPP_LOG_GLOBAL_CONF.logPrefixFile = sPrefixLogFile;
  WsjcppLog::g_WSJCPP_LOG_GLOBAL_CONF.doLogRotateUpdateFilename(true);
}

void WsjcppLog::setEnableLogFile(bool bEnable) {
  WsjcppLog::g_WSJCPP_LOG_GLOBAL_CONF.enableLogFile = bEnable;
}

void WsjcppLog::setRotationPeriodInSec(long nRotationPeriodInSec) {
  WsjcppLog::g_WSJCPP_LOG_GLOBAL_CONF.logRotationPeriodInSeconds = nRotationPeriodInSec;
}

void WsjcppLog::add(WsjcppColorModifier &clr, const std::string &sType, const std::string &sTag,
                    const std::string &sMessage) {
  WsjcppLog::g_WSJCPP_LOG_GLOBAL_CONF.doLogRotateUpdateFilename();

  std::lock_guard<std::mutex> lock(WsjcppLog::g_WSJCPP_LOG_GLOBAL_CONF.logMutex);
  WsjcppColorModifier def(WsjcppColorCode::FG_DEFAULT);

  std::string sLogMessage = WsjcppCore::getCurrentTimeForLogFormat() + ", " + WsjcppCore::getThreadId() + " [" + sType +
                            "] " + sTag + ": " + sMessage;
  std::cout << clr << sLogMessage << def << std::endl;

  WsjcppLog::g_WSJCPP_LOG_GLOBAL_CONF.logLastMessages.push_front(sLogMessage);

  while (WsjcppLog::g_WSJCPP_LOG_GLOBAL_CONF.logLastMessages.size() > 50) {
    WsjcppLog::g_WSJCPP_LOG_GLOBAL_CONF.logLastMessages.pop_back();
  }

  // log file
  if (WsjcppLog::g_WSJCPP_LOG_GLOBAL_CONF.enableLogFile) {
    std::ofstream logFile(WsjcppLog::g_WSJCPP_LOG_GLOBAL_CONF.logFile, std::ios::app);
    if (!logFile) {
      std::cout << "Error Opening File" << std::endl;
      return;
    }

    logFile << sLogMessage << std::endl;
    logFile.close();
  }
}

// ---------------------------------------------------------------------
// WsjcppResourceFile

WsjcppResourceFile::WsjcppResourceFile() {
  WsjcppResourcesManager::add(this);
}

// ---------------------------------------------------------------------
// WsjcppResourcesManager

std::vector<WsjcppResourceFile *> *g_pWsjcppResourceFiles = nullptr;

void WsjcppResourcesManager::initGlobalVariables() {
  if (g_pWsjcppResourceFiles == nullptr) {
    g_pWsjcppResourceFiles = new std::vector<WsjcppResourceFile *>();
  }
}

void WsjcppResourcesManager::add(WsjcppResourceFile *pStorage) {
  WsjcppResourcesManager::initGlobalVariables();
  g_pWsjcppResourceFiles->push_back(pStorage);
}

bool WsjcppResourcesManager::has(const std::string &sFilename) {
  WsjcppResourcesManager::initGlobalVariables();
  for (int i = 0; i < WsjcppResourcesManager::list().size(); i++) {
    if (WsjcppResourcesManager::list()[i]->getFilename() == sFilename) {
      return true;
    }
  }
  return false;
}

WsjcppResourceFile *WsjcppResourcesManager::get(const std::string &sFilename) {
  WsjcppResourcesManager::initGlobalVariables();
  for (int i = 0; i < WsjcppResourcesManager::list().size(); i++) {
    if (WsjcppResourcesManager::list()[i]->getFilename() == sFilename) {
      return WsjcppResourcesManager::list()[i];
    }
  }
  return nullptr;
}

const std::vector<WsjcppResourceFile *> &WsjcppResourcesManager::list() {
  return *g_pWsjcppResourceFiles;
}

/*
bool WsjcppResourcesManager::make(const std::string &sWorkspace) {
  if (!WsjcppResourcesManager::createFolders(sWorkspace)) {
    return false;
  }
  return WsjcppResourcesManager::extractFiles(sWorkspace);
}

bool WsjcppResourcesManager::createFolders(const std::string &sWorkspace) {
  // prepare folders
  std::vector<std::string> vCreateDirs;
  vCreateDirs.push_back(sWorkspace + "/logs");
  vCreateDirs.push_back(sWorkspace + "/teams");
  vCreateDirs.push_back(sWorkspace + "/checkers");
  vCreateDirs.push_back(sWorkspace + "/html");
  vCreateDirs.push_back(sWorkspace + "/html/css");
  vCreateDirs.push_back(sWorkspace + "/html/js");
  vCreateDirs.push_back(sWorkspace + "/html/images");
  vCreateDirs.push_back(sWorkspace + "/html/images/teams");
  vCreateDirs.push_back(sWorkspace + "/html/images/states");

  for(int i = 0; i < vCreateDirs.size(); i++) {
    std::string sPath = vCreateDirs[i];
    // check dir existing
    if (!FS::dirExists(sPath)) {
      // try make dir
      if (!FS::makeDir(sPath)) {
        std::cout << "Could not create folder " << sPath << std::endl;
        return false;
      } else {
        std::cout << "Created folder " << sPath << std::endl;
      }
    }
  }
  return true;
}

bool WsjcppResourcesManager::extractFiles(const std::string &sWorkspace) {
  // TODO mkdir -p for files
  const std::vector<WsjcppResourceFile*> list = WsjcppResourcesManager::list();
  for(int i = 0; i < list.size(); i++) {
    std::string sFilename = sWorkspace + "/" + list[i]->filename();
    if (!FS::fileExists(sFilename)) {
      if (!FS::writeFile(sFilename, list[i]->buffer(), list[i]->bufferSize())) {
        std::cout << "Could not write file " << sFilename << std::endl;
        return false;
      } else {
        std::cout << "Created file " << sFilename << std::endl;
      }
    }
  }
  return true;
}
*/

namespace wsjcpp {

const std::string &Core::englishAlphabetLowerCase() {
  static const std::string ret = "abcdefghijklmnopqrstuvwxyz";
  return ret;
}

const std::string &Core::englishAlphabetUpperCase() {
  static const std::string ret = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  return ret;
}

const std::string &Core::englishAlphabetBothCase() {
  static const std::string ret = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  return ret;
}

const std::string &Core::englishAlphabetBothCaseAndNumbers() {
  static const std::string ret = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  return ret;
}

std::string Core::randomString(const std::string &alphabet, int length) {
  std::string ret = "";
  ret.resize(length, alphabet[0]);
  int alphabet_len = alphabet.length();
  for (int i = 0; i < length; i++) {
    ret[i] = alphabet[rand() % alphabet_len];
  }
  return ret;
}

std::string normalize_filepath(const std::string &path) {
  // split path by /
  std::vector<std::string> vNames;
  bool has_root = false;
  {
    std::string fname = "";
    size_t nStrLen = path.length();
    for (size_t i = 0; i < path.length(); i++) {
      if (path[i] == '/') {
        vNames.push_back(fname);
        if (i == 0) {
          has_root = true;
        }
        fname = "";
        if (i == nStrLen - 1) {
          vNames.push_back("");
        }
      } else {
        fname += path[i];
      }
    }
    if (fname != "") {
      vNames.push_back(fname);
    }
  }

  // filtered
  size_t nLen = vNames.size();
  std::vector<std::string> result;
  for (size_t i = 0; i < nLen; i++) {
    std::string cur = vNames[i];
    if (cur == "" && i == nLen - 1) {
      result.push_back(cur);
      continue;
    }
    if (cur == ".." && result.size() == 1 && has_root) {
      continue;
    }
    if ((cur == "" || cur == ".") && i != 0) {
      continue;
    }
    if (cur == ".." && result.size() > 0) {
      std::string sPrev = result[result.size() - 1];
      if (sPrev == "") {
        result.pop_back();
        result.push_back(cur);
      } else if (sPrev != "." && sPrev != "..") {
        result.pop_back();
      } else {
        result.push_back(cur);
      }
    } else {
      result.push_back(cur);
    }
  }

  std::string ret = "";
  size_t nNewLen = result.size();
  size_t nLastNew = nNewLen - 1;
  for (size_t i = 0; i < nNewLen; i++) {
    ret += result[i];
    if (i != nLastNew) {
      ret += "/";
    }
  }
  return ret;
}

std::string extract_url_protocol(const std::string &value) {
  std::string ret = "";
  size_t nPosProtocol = value.find("://");
  if (nPosProtocol == std::string::npos) {
    return ret;
  }
  ret = value.substr(0, nPosProtocol);
  return ret;
}

bool recursive_copy_files(const std::string &source_dir, const std::string &target_dir, std::string &error) {
  if (!wsjcpp::dir_exists(source_dir)) {
    WsjcppLog::err("recursive_copy_files", "Source Dir '" + source_dir + "' did not exists");
    return false;
  }

  if (!wsjcpp::dir_exists(target_dir)) {
    if (!WsjcppCore::makeDir(target_dir)) {
      WsjcppLog::err("recursive_copy_files", "Could not create target dir '" + target_dir + "'");
      return false;
    }
  }

  std::vector<std::string> found_files = WsjcppCore::getListOfFiles(source_dir);
  for (int i = 0; i < found_files.size(); i++) {
    std::string source_file = source_dir + "/" + found_files[i];
    std::string target_file = target_dir + "/" + found_files[i];
    if (!WsjcppCore::copyFile(source_file, target_file)) {
      error = "Could not copy file '" + source_file + "' -> '" + target_file + "'";
      return false;
    }
  }

  std::vector<std::string> found_dirs = wsjcpp::directory_list(source_dir);
  for (int i = 0; i < found_dirs.size(); i++) {
    std::string next_source_dir = source_dir + "/" + found_dirs[i];
    std::string next_target_dir = target_dir + "/" + found_dirs[i];
    if (!wsjcpp::dir_exists(next_target_dir)) {
      if (!WsjcppCore::makeDir(next_target_dir)) {
        error = "Could not create target subdir '" + next_target_dir + "'";
        return false;
      }
    }

    if (!wsjcpp::recursive_copy_files(next_target_dir, next_target_dir, error)) {
      return false;
    }
  }
  return true;
}

bool recursive_remove_dir(const std::string &dirpath, std::string &error) {
  if (!wsjcpp::dir_exists(dirpath)) {
    error = "Dir '" + dirpath + "' did not exists";
    return false;
  }
  std::vector<std::string> vFiles = WsjcppCore::getListOfFiles(dirpath);
  for (int i = 0; i < vFiles.size(); i++) {
    std::string filepath = dirpath + "/" + vFiles[i];
    if (!WsjcppCore::removeFile(filepath)) {
      error = "File '" + filepath + "' did not removed";
      return false;
    }
  }
  std::vector<std::string> vDirs = wsjcpp::directory_list(dirpath);
  for (int i = 0; i < vDirs.size(); i++) {
    std::string sDir2 = dirpath + "/" + vDirs[i];
    if (!recursive_remove_dir(sDir2, error)) {
      return false;
    }
  }
  if (!WsjcppCore::removeFile(dirpath)) {
    error = "File '" + dirpath + "' did not removed";
    return false;
  }
  return true;
}

bool starts_with(const std::string &str, const std::string &start_str) {
  return str.rfind(start_str, 0) == 0;
}

bool ends_with(const std::string &str, const std::string &end_str) {
  // https://www.techiedelight.com/check-if-a-string-ends-with-another-string-in-cpp/
  if (str.length() < end_str.length()) {
    return false;
  }
  return std::equal(end_str.rbegin(), end_str.rend(), str.rbegin());
}

std::vector<std::string> directory_list(const std::string &dirpath) {
  std::vector<std::string> ret;
  std::string _dirpath = wsjcpp::normalize_filepath(dirpath);
  if (_dirpath.size() > 0 && _dirpath[_dirpath.size() - 1] == '/') {
    _dirpath = _dirpath.substr(0, _dirpath.size() - 1);
  }
  if (!wsjcpp::dir_exists(_dirpath)) {
    return ret;
  }
  for (auto &entry : std::filesystem::directory_iterator(_dirpath)) {
    if (entry.is_directory()) {
      std::string path = entry.path();
      path.erase(0, _dirpath.size() + 1);
      ret.push_back(path);
    }
  }
  std::sort(ret.begin(), ret.end());
  return ret;
}

std::string generate_uuid() {
  std::string ret = "00000000-0000-0000-0000-000000000000";
  const std::string sAlphabet = "0123456789abcdef";
  for (int i = 0; i < 36; i++) {
    if (i != 8 && i != 13 && i != 18 && i != 23) {
      ret[i] = sAlphabet[rand() % sAlphabet.length()];
    }
  }
  return ret;
}

std::string parent_dirpath(const std::string &filepath) {
  std::string _filepath = wsjcpp::normalize_filepath(filepath);
  if (_filepath.size() > 0 && _filepath[_filepath.size() - 1] == '/') {
    _filepath = _filepath.substr(0, _filepath.size() - 1);
  }
  std::string filename = wsjcpp::extract_filename(_filepath);
  std::string _ret = _filepath.substr(0, _filepath.length() - filename.length());
  if (_ret.size() > 0 && _ret[_ret.size() - 1] == '/') {
    _ret = _ret.substr(0, _ret.size() - 1);
  }
  return _ret;
}

std::string to_snake_case(const std::string &name) {
  std::string _to_lower_and_underscore = "";
  for (int i = 0; i < name.size(); i++) {
    char c = name[i];
    if (c >= 'A' && c <= 'Z') {
      _to_lower_and_underscore += '_';
      _to_lower_and_underscore += char(c + 0x20); // to lower
    } else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
      _to_lower_and_underscore += c;
    } else {
      _to_lower_and_underscore += '_';
    }
  }
  // TODO: ä -> ae, ö -> oe, ü -> ue

  std::string ret;
  int underscore_counter = 0;
  for (int i = 0; i < _to_lower_and_underscore.size(); i++) {
    char c = _to_lower_and_underscore[i];
    if (c == '_') {
      underscore_counter++;
      if (underscore_counter == 1) {
        ret += c;
      }
    } else {
      underscore_counter = 0;
      ret += c;
    }
  }

  // removing first underscore if next charater is not number
  if (ret.size() > 1 && ret[0] == '_' && (ret[1] < '0' || ret[1] > '9')) {
    ret = ret.substr(1, ret.size() - 1);
  }
  if (ret.size() > 0 && ret[ret.size() - 1] == '_') {
    ret = ret.substr(0, ret.size() - 1);
  }
  return ret;
}

bool dir_exists(const std::string &dir_path) {
  struct stat st;
  bool exists = (stat(dir_path.c_str(), &st) == 0);
  if (exists) {
    return (st.st_mode & S_IFDIR) != 0;
  }
  return false;
}

bool file_exists(const std::string &file_path) {
  struct stat st;
  bool exists = (stat(file_path.c_str(), &st) == 0);
  if (exists) {
    return (st.st_mode & S_IFDIR) == 0;
  }
  return false;
}

std::vector<std::string> split(const std::string &source, const std::string &delimiter) {
  std::vector<std::string> ret;
  size_t pos = 0;
  size_t len = source.length();
  size_t delimiter_len = delimiter.length();
  while (pos < len) {
    std::size_t next_pos = source.find(delimiter, pos);
    if (next_pos != std::string::npos) {
      std::string token = source.substr(pos, next_pos - pos);
      ret.push_back(token);
      pos = next_pos + delimiter_len;
      if (next_pos + delimiter_len == len) { // last delimiter
        ret.push_back("");
      }
    } else {
      std::string token = source.substr(pos, len - pos);
      ret.push_back(token);
      break;
    }
  }
  return ret;
}

std::string join(const std::vector<std::string> &source, const std::string &delimiter) {
  std::string ret;
  for (int i = 0; i < source.size(); i++) {
    if (i != 0) {
      ret += delimiter;
    }
    ret += source[i];
  }
  return ret;
}

void replace_all_in(std::string &target, const std::string &from, const std::string &to) {
  if (from.empty()) {
    return;
  }
  size_t start_pos = 0;
  while ((start_pos = target.find(from, start_pos)) != std::string::npos) {
    target.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'sFrom', like replacing 'x' with 'yx'
  }
}

bool create_empty_file(const std::string &filepath) {
  if (wsjcpp::file_exists(filepath)) {
    return false;
  }
  std::ofstream f(filepath, std::ios::out | std::ios::binary);
  if (!f) {
    std::cerr << "FAILED could not create file to write " << filepath << std::endl;
    return false;
  }
  f.close();
  return true;
}

std::string extract_filename(const std::string &filepath) {
  // split path by /
  std::vector<std::string> names;
  std::string s = "";
  size_t nStrLen = filepath.length();
  for (size_t i = 0; i < filepath.length(); i++) {
    if (filepath[i] == '/') {
      names.push_back(s);
      s = "";
      if (i == nStrLen - 1) {
        names.push_back("");
      }
    } else {
      s += filepath[i];
    }
  }
  if (s != "") {
    names.push_back(s);
  }
  std::string ret;
  if (names.size() > 0) {
    ret = names[names.size() - 1];
  }
  return ret;
}

int levenshtein_distance(const std::string &str1, const std::string &str2) {
  const std::size_t len1 = str1.length();
  const std::size_t len2 = str2.length();
  std::vector<std::vector<unsigned int>> d(len1 + 1, std::vector<unsigned int>(len2 + 1));

  d[0][0] = 0;
  for (unsigned int i = 1; i <= len1; ++i) {
    d[i][0] = i;
  }
  for (unsigned int i = 1; i <= len2; ++i) {
    d[0][i] = i;
  }

  for (unsigned int i = 1; i <= len1; ++i) {
    for (unsigned int j = 1; j <= len2; ++j) {
      d[i][j] = std::min(std::min(d[i - 1][j] + 1, d[i][j - 1] + 1),
                         (d[i - 1][j - 1] + (str1[i - 1] == str2[j - 1] ? 0 : 1)));
    }
  }
  return d[len1][len2];
}

bool user_is_root() {
  // Root always has an Effective User ID (EUID) of 0
  return geteuid() == 0;
}

bool change_current_process_privileges(int user_id, std::string &error) {
  if (setgid(user_id) != 0) {
    error = "Failed to set GID";
    return false;
  }
  if (setuid(user_id) != 0) {
    error = "Failed to set UID";
    return false;
  }
  if (setuid(0) == 0) {
    error = "Security Risk: Privileges were not permanently dropped!";
    return false;
  }
  if (getuid() != user_id) {
    error = "NOT CHANGED";
    return false;
  }
  // Successful changed privileges.
  return true;
}

operation_system operation_system_platform() {
#ifdef _WIN32
  return operation_system::WINDOWS;
#endif
#if defined(_MSC_VER)
  return operation_system::WINDOWS;
#endif
#ifdef __unix__
  return operation_system::LINUX;
#endif
  return operation_system::UNKNOWN;
}

bool is_linux() {
  return operation_system_platform() == operation_system::LINUX;
}

bool is_windows() {
  return operation_system_platform() == operation_system::WINDOWS;
}

std::string padding_right(const std::string &source, char pad_char, size_t length) {
  std::string ret;
  if (length > source.length()) {
    size_t pad_len = length - source.length();
    ret.reserve(pad_len);
    for (size_t i = 0; i < pad_len; i++) {
      ret += pad_char;
    }
  }
  return source + ret;
}

std::string padding_left(const std::string &source, char pad_char, size_t length) {
  std::string ret;
  if (length > source.length()) {
    size_t pad_len = length - source.length();
    ret.reserve(pad_len);
    for (size_t i = 0; i < pad_len; i++) {
      ret += pad_char;
    }
  }
  return ret + source;
}

bool is_valid_ip4(const std::string &value, std::string &error) {
  int n = 0;
  std::string s[4] = {"", "", "", ""};
  for (int i = 0; i < value.length(); i++) {
    char c = value[i];
    if (n > 3) {
      error = "Groups number must be less than 5 (like '0.0.0.0')";
      return false;
    }
    if (c >= '0' && c <= '9') {
      s[n] += c;
    } else if (c == '.') {
      n++;
    } else {
      error = "Unexpected character '";
      error += c;
      error += "'";
      return false;
    }
  }
  for (int i = 0; i < 4; i++) {
    if (s[i].length() > 3) {
      error = "Value '" + s[i] + "' could not contains more than 3 digits in a row";
      return false;
    }
    int p = std::stoi(s[i]);
    if (p > 255 || p < 0) {
      error = "Value '" + std::to_string(p) + "' must be 0..255";
      return false;
    }
  }
  return true;
}

bool is_valid_ip6(const std::string &value, std::string &error) {
  // TODO redesign without arpa
  unsigned char buf[sizeof(struct in6_addr)];
  bool isValid = inet_pton(AF_INET6, value.c_str(), buf);
  if (!isValid) {
    error = "Format of ipv6 invalid";
  }
  return isValid;
}

bool is_valid_date(const std::string &value, std::string &error) {
  int nSize = value.size();
  if (nSize != 10) {
    error = "Invalid size format expected length 10";
    return false;
  }

  for (int i = 0; i < 10; i++) {
    char c = value[i];
    if (i == 4 || i == 7) {
      if (c != '-') {
        error = "Expected '-' in " + std::to_string(i) + " position, but got '";
        error += c;
        error += "'";
        return false;
      }
      continue;
    }
    if (c < '0' || c > '9') {
      error = "Unexpected char '";
      error += c;
      error += "' in " + std::to_string(i) + " position";
      return false;
    }
  }
  // 2020-01-01
  std::string sYear = value.substr(0, 4);
  int nYear = std::atoi(sYear.c_str());

  std::string sMonth = value.substr(5, 2);
  int nMonth = std::atoi(sMonth.c_str());
  if (nMonth < 1 || nMonth > 12) {
    error = "Invalid value number of month '" + std::to_string(nMonth) + "' expected 01..12";
    return false;
  }

  int nMaxDay = 0;
  if (nMonth == 1 || nMonth == 3 || nMonth == 5 || nMonth == 7 || nMonth == 8 || nMonth == 10 || nMonth == 12) {
    nMaxDay = 31;
  } else if (nMonth == 4 || nMonth == 6 || nMonth == 9 || nMonth == 11) {
    nMaxDay = 30;
  } else if (nMonth == 2 && nYear % 4 == 0) {
    nMaxDay = 29;
  } else if (nMonth == 2 && nYear % 4 != 0) {
    nMaxDay = 28;
  }

  std::string sDay = value.substr(8, 2);
  int nDay = std::atoi(sDay.c_str());
  if (nDay < 1 || nDay > nMaxDay) {
    error = "Invalid value number of day '" + std::to_string(nDay) + "' expected 01.." + std::to_string(nMaxDay);
    return false;
  }
  return true;
}

bool is_valid_time24(const std::string &value, std::string &error) {
  int nSize = value.size();
  if (nSize != 8) {
    error = "Invalid size format expected length 8";
    return false;
  }

  for (int i = 0; i < 8; i++) {
    char c = value[i];
    if (i == 2 || i == 5) {
      if (c != ':') {
        error = "Expected ':' in " + std::to_string(i) + " position, but got '";
        error += c;
        error += "'";
        return false;
      }
      continue;
    }
    if (c < '0' || c > '9') {
      error = "Unexpected char '";
      error += c;
      error += "' in " + std::to_string(i) + " position";
      return false;
    }
  }

  std::string sHours = value.substr(0, 2);
  int nHours = std::atoi(sHours.c_str());
  if (nHours > 23) {
    error = "Invalid value of hours '" + std::to_string(nHours) + "' expected 00..23";
    return false;
  }
  std::string sMinutes = value.substr(3, 2);
  int nMinutes = std::atoi(sMinutes.c_str());
  if (nMinutes > 59) {
    error = "Invalid value of minutes '" + std::to_string(nMinutes) + "' expected 00..59";
    return false;
  }

  std::string sSeconds = value.substr(6, 2);
  int nSeconds = std::atoi(sSeconds.c_str());
  if (nSeconds > 59) {
    error = "Invalid value of seconds '" + std::to_string(nSeconds) + "' expected 00..59";
    return false;
  }
  return true;
}

bool is_valid_domain_name(const std::string &value, std::string &error) {
  std::vector<std::string> vSubDomains;
  std::string sTmpDomain = "";
  int nAddressLen = value.size();
  char cPrev = 0;
  for (int i = 0; i < nAddressLen; i++) {
    char c = value[i];
    if (i == 0 && c == '.') {
      error = "Domain Name '" + value + "' could not be start on '.'";
      return false;
    }
    if (c == '.') {
      if (sTmpDomain != "") {
        vSubDomains.push_back(sTmpDomain);
        sTmpDomain = "";
        continue;
      } else {
        error = "Domain Name '" + value + "' could not contains '..'";
        return false;
      }
    }
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-') {
      sTmpDomain += c;
      if (cPrev == '-' && c == cPrev) {
        error = "Domain Name '" + value + "' could not two times in a row '";
        error += c;
        error += c;
        error += "'";
        return false;
      }
      cPrev = c;
    } else {
      error = "Domain Name '" + value + "' contains unexpected '";
      error += c;
      error += "'";
      return false;
    }
  }

  if (sTmpDomain != "") {
    vSubDomains.push_back(sTmpDomain);
    sTmpDomain = "";
  } else {
    error = "Domain Name '" + value + "' could not contains '.' on end";
    return false;
  }

  if (vSubDomains.size() < 2) {
    error = "Domain Name '" + value + "' must contains least one dot";
    return false;
  }
  std::string sRootDomain = vSubDomains[vSubDomains.size() - 1];
  if (sRootDomain.size() < 2) {
    error = "Domain Name '" + value + "' has wrong root domain '" + sRootDomain + "' length must be more then 1";
    return false;
  }
  for (int i = 0; i < sRootDomain.size(); i++) {
    char c = sRootDomain[i];
    if ((c < 'A' || c > 'Z') && (c < 'a' || c > 'z')) {
      error = "Domain Name '" + value + "' has wrong root domain '" + sRootDomain + "' must have only chars";
      return false;
    }
  }

  for (int i = 0; i < vSubDomains.size(); i++) {
    std::string sDomain = vSubDomains[i];
    char c = sDomain[0];
    if ((c < 'A' || c > 'Z') && (c < 'a' || c > 'z') && (c < '0' || c > '9')) {
      error = "Subdomain '" + sDomain + "' could not start on '";
      error += c;
      error += "'";
      return false;
    }
    c = sDomain[sDomain.size() - 1];
    if ((c < 'A' || c > 'Z') && (c < 'a' || c > 'z') && (c < '0' || c > '9')) {
      error = "Subdomain '" + sDomain + "' could not end on '";
      error += c;
      error += "'";
      return false;
    }
  }
  return true;
}

bool is_valid_port(const std::string &value, std::string &error) {
  int nPort = std::atoi(value.c_str());
  return wsjcpp::is_valid_port(nPort, error);
}

bool is_valid_port(int nValue, std::string &error) {
  if (nValue < 1 || nValue > 65535) {
    error = "Port '" + std::to_string(nValue) + "' must be more then 0 and less then 65536";
    return false;
  }
  return true;
}

bool is_valid_url_protocol(const std::string &value, std::string &error) {
  if (value != "http" && value != "https" && value != "ws" && value != "wss" && value != "ftp" && value != "ssl") {
    error = "Unexpected protocol '" + value + "'";
    return false;
  }
  return true;
}

bool is_valid_base64(const std::string &value, std::string &error) {
  int nSize = value.size();
  if (nSize % 4 != 0) {
    error = "Value size must be a multiple of 4";
    return false;
  }
  bool bLastChar = false;
  for (int i = 0; i < nSize; i++) {
    char c = value[i];
    if (!bLastChar && c == '=') {
      bLastChar = true;
      continue;
    }
    if (bLastChar && c == '=') {
      continue;
    }

    if (bLastChar && c != '=') {
      error = "Unexpected char '";
      error += c;
      error += "' after '=' in " + std::to_string(i) + " position";
      return false;
    }

    if ((c < 'A' || c > 'Z') && (c < 'a' || c > 'z') && (c < '0' || c > '9') && c != '+' && c != '/') {
      error = "Unexpected char '";
      error += c;
      error += "' in " + std::to_string(i) + " position";
      return false;
    }
  }
  return true;
}

// helpful validators

validator_date::validator_date() : validator<std::string>("date") {
}

bool validator_date::is_valid(const std::string &value, std::string &error) {
  return wsjcpp::is_valid_date(value, error);
}

validator_regexp::validator_regexp(const std::string &name, const std::string &_regexp) : validator<std::string>(name) {
  m_sValidator = _regexp;
  m_rxValidator = std::regex(_regexp);
}

bool validator_regexp::is_valid(const std::string &value, std::string &error) {
  if (!std::regex_match(value, m_rxValidator)) {
    error = name() + " - Value must match regular expression " + m_sValidator;
    return false;
  }
  return true;
}

validator_str_list::validator_str_list(const std::string &sTypeName, const std::vector<std::string> &vListValues)
    : validator<std::string>(sTypeName) {
  m_vListValues = vListValues;
}

bool validator_str_list::is_valid(const std::string &value, std::string &error) {
  if (std::find(m_vListValues.begin(), m_vListValues.end(), value) != m_vListValues.end()) {
    return true;
  }
  error = name() + " expected one of [";
  for (int i = 0; i < m_vListValues.size(); i++) {
    error += "'" + m_vListValues[i] + "'";
    if (i < m_vListValues.size() - 1) {
      error += ", ";
    }
  }
  error += "]";
  return false;
}

validator_email::validator_email()
    : validator_regexp("email", "^[0-9a-zA-Z]{1}[0-9a-zA-Z-._]*[0-9a-zA-Z]{1}@[0-9a-zA-Z]{1}"
                                "[-.0-9a-zA-Z]*[0-9a-zA-Z]{1}\\.[a-zA-Z]{2,6}$") {
}

validator_uuid::validator_uuid()
    : validator_regexp("uuid", "^[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-"
                               "f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}$") {
}

validator_strlen::validator_strlen(int min_length, int max_length) : validator<std::string>("string_length") {
  m_min_length = min_length;
  m_max_length = max_length;
}

bool validator_strlen::is_valid(const std::string &value, std::string &error) {
  if (value.length() < m_min_length) {
    error = "Value must have more than " + std::to_string(m_min_length) + " symbols";
    return false;
  }

  if (value.length() > m_max_length) {
    error = "Value must have less than " + std::to_string(m_max_length) + " symbols";
    return false;
  }
  return true;
}

validator_jwt::validator_jwt() : validator_regexp("jwt", "^[A-Za-z0-9-_=]+\\.[A-Za-z0-9-_=]+\\.?[A-Za-z0-9-_.+/=]*$") {
}

validator_time24::validator_time24() : validator<std::string>("time_h24") {
}

bool validator_time24::is_valid(const std::string &value, std::string &error) {
  return wsjcpp::is_valid_time24(value, error);
}

validator_datetime::validator_datetime() : validator<std::string>("datetime") {
}

bool validator_datetime::is_valid(const std::string &value, std::string &error) {
  int nSize = value.size();
  // '2020-01-01T00:00:00'
  if (nSize != 19) {
    error = "Invalid size format expected length 19";
    return false;
  }
  std::string sDate = value.substr(0, 10);
  if (!wsjcpp::is_valid_date(sDate, error)) {
    return false;
  }
  if (value[10] != 'T') {
    error = "Expected 'T' in 10 position, but got '";
    error += value[10];
    error += "'";
    return false;
  }
  std::string sTime = value.substr(11, 8);
  if (!wsjcpp::is_valid_time24(sTime, error)) {
    return false;
  }
  return true;
}

validator_url::validator_url() : validator<std::string>("url") {
  m_rx_ip4_format = std::regex("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$");
}

bool validator_url::is_valid(const std::string &value, std::string &error) {
  if (value.size() == 0) {
    error = "Value is empty";
    return false;
  }
  if (value.find(".") == std::string::npos) {
    error = "Must contain at least one dot";
    return false;
  }
  std::string sProtocol = wsjcpp::extract_url_protocol(value);
  if (!wsjcpp::is_valid_url_protocol(sProtocol, error)) {
    return false;
  }

  int nStartPos = sProtocol.length() + 3;
  std::string sAuthorityAddressPath = "";
  for (int i = nStartPos; i < value.size(); i++) {
    char c = value[i];
    if (c == '?') {
      break;
    }
    sAuthorityAddressPath += c;
  }
  std::string sQuery = value.substr(sProtocol.length() + 3 + sAuthorityAddressPath.size());
  std::string sAddressAndPath = sAuthorityAddressPath;

  int nPosAuthority = sAuthorityAddressPath.find("@");
  std::string sAuthority = "";
  if (nPosAuthority != std::string::npos) {
    sAuthority = sAuthorityAddressPath.substr(0, nPosAuthority);
    sAddressAndPath = sAuthorityAddressPath.substr(nPosAuthority + 1);
  }
  if (sAuthority != "") {
    // error = "TODO check username and password sAuthority= [" + sAuthority +
    // "]";
    // -.~_!$&'()*+,;=:%40:80%2f::::::
    // WsjcppLog::warn(TAG, error);
    // return false;
  }
  std::string sAddress = sAddressAndPath;
  std::string sPath = "";
  int nPosAddress = sAddressAndPath.find("/");
  if (nPosAddress != std::string::npos) {
    sAddress = sAddressAndPath.substr(0, nPosAddress);
    sPath = sAddressAndPath.substr(nPosAddress);
  }

  if (sAddress.size() == 0) {
    error = "Address could not be empty";
    return false;
  }

  int nPosPort = sAddress.find(":");
  std::string sPort = "";
  if (sAddress.find(":") != std::string::npos) {
    sPort = sAddress.substr(nPosPort + 1);
    sAddress = sAddress.substr(0, nPosPort);
  }

  if (sPort != "") {
    if (!wsjcpp::is_valid_port(sPort, error)) {
      return false;
    }
  }

  if (std::regex_match(sAddress, m_rx_ip4_format)) {
    if (!wsjcpp::is_valid_ip4(sAddress, error)) {
      return false;
    }
    if (sPort != "") {
      int nPort = std::atoi(sPort.c_str());
      if (nPort < 1 || nPort > 65535) {
        error = "Port '" + std::to_string(nPort) + "' must be more then 0 and less then 65536";
        return false;
      }
    }
  } else {
    if (!wsjcpp::is_valid_domain_name(sAddress, error)) {
      return false;
    }
  }

  if ((sPath == "" || sPath == "/") && sQuery == "") {
    return true;
  }

  if (sPath != "") {
    for (int i = 0; i < sPath.length(); i++) {
      char c = sPath[i];
      if (c == ' ') {
        error = "Path could not contains whitespace ' '";
        return false;
      }
    }
  }

  if (sQuery != "") {
    for (int i = 0; i < sQuery.length(); i++) {
      char c = sQuery[i];
      if (c == ' ') {
        error = "Query could not contains whitespace ' ' (must be encoded)";
        return false;
      }
    }
  }

  // error = "sAddressAndPath=[" + sAddressAndPath + "], , sAddress=[" +
  // sAddress + "]";
  return true;
}

validator_domain_name::validator_domain_name() : validator<std::string>("domain_name") {
}

bool validator_domain_name::is_valid(const std::string &value, std::string &error) {
  return wsjcpp::is_valid_domain_name(value, error);
}

validator_base64::validator_base64() : validator<std::string>("base64") {
}

bool validator_base64::is_valid(const std::string &value, std::string &error) {
  return wsjcpp::is_valid_base64(value, error);
}

validator_int::validator_int() : validator<std::string>("int") {
}

bool validator_int::is_valid(const std::string &value, std::string &error) {
  int nSize = value.size();
  bool bHasOneAndMoreNumbers = false;
  for (int i = 0; i < nSize; i++) {
    char c = value[i];
    if (c == '-' && i == 0) {
      continue;
    }
    if (c < '0' || c > '9') {
      error = "Unexpected char '";
      error += c;
      error += "' in " + std::to_string(i) + " position";
      return false;
    }
    bHasOneAndMoreNumbers = true;
  }
  return bHasOneAndMoreNumbers;
}

validator_hex::validator_hex() : validator<std::string>("hex") {
}

bool validator_hex::is_valid(const std::string &value, std::string &error) {
  int nSize = value.size();
  if (nSize == 0) {
    error = "Empty string";
    return false;
  }
  for (int i = 0; i < nSize; i++) {
    char c = value[i];
    if (c == '-' && i == 0) {
      continue;
    }
    if ((c < '0' || c > '9') && (c < 'a' || c > 'f') && (c < 'A' || c > 'F')) {
      error = "Unexpected char '";
      error += c;
      error += "' in " + std::to_string(i) + " position";
      return false;
    }
  }
  return true;
}

validator_int_min::validator_int_min(int min_value) : validator<int>("int_min") {
  m_min_value = min_value;
}

bool validator_int_min::is_valid(const int &value, std::string &error) {
  if (value < m_min_value) {
    error = "Value must be more or equal then " + std::to_string(m_min_value);
    return false;
  }
  return true;
}

validator_int_max::validator_int_max(int max_value) : validator<int>("int_max") {
  m_max_value = max_value;
}

bool validator_int_max::is_valid(const int &value, std::string &error) {
  if (value > m_max_value) {
    error = "Value must be less or equal then " + std::to_string(m_max_value);
    return false;
  }
  return true;
}

} // namespace wsjcpp