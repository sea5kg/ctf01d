/**********************************************************************************
 * Copyright (c) 2020-2026 Evgenii Sopov <mrseakg@gmail.com>
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

// Original repository: https://github.com/wsjcpp/wsjcpp-employees

#pragma once

#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <stdexcept>

#define WSJCPP_EMPLOYEES_VERSION_0_2_1
#define WSJCPP_EMPLOYEES_VERSION_0_2
#define WSJCPP_EMPLOYEES_VERSION_0

// ---------------------------------------------------------------------
// base employ class

class WsjcppEmployBase {
public:
  WsjcppEmployBase(const std::vector<std::string> &vNames, const std::vector<std::string> &vLoadAfter);
  virtual ~WsjcppEmployBase();
  virtual bool init(const std::string &sName, bool bSilent) = 0;
  virtual bool deinit(const std::string &sName, bool bSilent) = 0;
  const std::vector<std::string> &loadAfter();

private:
  std::string TAG;
  std::vector<std::string> m_vNames;
  std::vector<std::string> m_vLoadAfter;
};

// ---------------------------------------------------------------------
// IWsjcppEmployeesLogger

class IWsjcppEmployeesLogger {
public:
  virtual void info(const std::string &tag, const std::string &message) = 0;
  virtual void ok(const std::string &tag, const std::string &message) = 0;
  virtual void warn(const std::string &tag, const std::string &message) = 0;
  virtual void err(const std::string &tag, const std::string &message) = 0;
  virtual void throw_err(const std::string &tag, const std::string &message) = 0;
};

extern IWsjcppEmployeesLogger *g_wsjcppEmployeesLogger;

// ---------------------------------------------------------------------
// public employees

extern std::map<std::string, WsjcppEmployBase *> *g_pWsjcppEmployees;
extern std::vector<std::string> *g_pWsjcppInitEmployees;

class WsjcppEmployees {
public:
  static void setLogger(IWsjcppEmployeesLogger *logger);
  static void initGlobalVariables();
  static void deinitGlobalVariables();
  static void addEmploy(const std::string &sName, WsjcppEmployBase *pEmploy);
  static bool init(const std::vector<std::string> &vLoadAfter, bool bSilent = false);
  static bool deinit(bool bSilent = false);
  static void recursiveTestDependencies(const std::vector<std::string> &v);
  static void logInfo(const std::string &tag, const std::string &message);
  static void logWarning(const std::string &tag, const std::string &message);
  static void logOk(const std::string &tag, const std::string &message);
  static void logError(const std::string &tag, const std::string &message);
  static void logThrowError(const std::string &tag, const std::string &message);
};

// ---------------------------------------------------------------------
// WsjcppEmployeesInit

struct WsjcppEmployeesInit {
  const bool initialized;
  bool uninitialized;
  bool silent;
  WsjcppEmployeesInit(const std::vector<std::string> &vLoadAfter, bool _silent = false)
    : silent(_silent), uninitialized(false), initialized(WsjcppEmployees::init(vLoadAfter, _silent)) {}
  ~WsjcppEmployeesInit() { this->deinit(); }
  bool deinit() {
    if (initialized && !uninitialized) {
      uninitialized = WsjcppEmployees::deinit(silent);
    }
    return uninitialized;
  }
};

// ---------------------------------------------------------------------
// RegistryEmploy
#define REGISTRY_WSJCPP_EMPLOY(classname)                                                                     \
  static classname *pWJSCppRegistryEmploy##classname = new classname();

// ---------------------------------------------------------------------
// findWsjcppEmploy

template <class T> T *findWsjcppEmploy(bool throw_on_error = true) {
  WsjcppEmployees::initGlobalVariables();
  std::string TAG = "findWsjcppEmploy";
  std::string sEmployName = T::name();
  WsjcppEmployBase *pEmploy = NULL;
  if (g_pWsjcppEmployees->count(sEmployName)) {
    pEmploy = g_pWsjcppEmployees->at(sEmployName);
  }
  if (pEmploy == NULL) {
    if (throw_on_error) {
      std::string err = "Not found employ " + sEmployName;
      throw std::runtime_error(err);
    }
    return NULL;
  }
  T *pTEmploy = dynamic_cast<T *>(pEmploy);
  if (pTEmploy == NULL) {
    if (throw_on_error) {
      std::string err = "Employ could not cast to T [" + sEmployName + "]";
      throw std::runtime_error(err);
    }
    return NULL;
  }
  return pTEmploy;
}

// ---------------------------------------------------------------------
// WsjcppEmployRuntimeGlobalCache

class WsjcppEmployRuntimeGlobalCache : public WsjcppEmployBase {
public:
  WsjcppEmployRuntimeGlobalCache();
  static std::string name() { return "WsjcppEmployRuntimeGlobalCache"; }
  virtual bool init(const std::string &sName, bool bSilent) override;
  virtual bool deinit(const std::string &sName, bool bSilent) override;
  void set(const std::string &sName, const std::string &sValue);
  bool has(const std::string &sName);
  std::string get(const std::string &sName);

private:
  std::string TAG;
  std::map<std::string, std::string> m_sStringMap;
};
