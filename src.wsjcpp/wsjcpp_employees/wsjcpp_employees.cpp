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

#include "wsjcpp_employees.h"
#include <algorithm>
#include <iostream>

std::map<std::string, WsjcppEmployBase *> *g_pWsjcppEmployees = nullptr;
std::vector<std::string> *g_pWsjcppInitEmployees = nullptr;
std::vector<std::string> *g_pWsjcppInitWith = nullptr;
IWsjcppEmployeesLogger *g_wsjcppEmployeesLogger = nullptr;

void WsjcppEmployees::setLogger(IWsjcppEmployeesLogger *logger) {
  g_wsjcppEmployeesLogger = logger;
}

void WsjcppEmployees::initGlobalVariables() {
  if (g_pWsjcppEmployees == nullptr) {
    // WsjcppEmployees::logInfo(std::string(), "Create employees map");
    g_pWsjcppEmployees = new std::map<std::string, WsjcppEmployBase *>();
  }
  if (g_pWsjcppInitEmployees == nullptr) {
    // WsjcppEmployees::logInfo(std::string(), "Create init employees vector");
    g_pWsjcppInitEmployees = new std::vector<std::string>();
  }
  if (g_pWsjcppInitWith == nullptr) {
    // WsjcppEmployees::logInfo(std::string(), "Create init employees vector");
    g_pWsjcppInitWith = new std::vector<std::string>();
  }
}

void WsjcppEmployees::deinitGlobalVariables() {
  const std::string TAG = "WsjcppEmployees::deinit";
  if (g_pWsjcppEmployees != nullptr) {
    std::map<std::string, WsjcppEmployBase *>::iterator it;
    for (it = g_pWsjcppEmployees->begin(); it != g_pWsjcppEmployees->end(); ++it) {
      std::string sEmployName = it->first;
      WsjcppEmployees::logOk(TAG, sEmployName + " ... UNREGISTERED");
      delete it->second;
      it->second = nullptr;
    }
    g_pWsjcppEmployees->clear();
    delete g_pWsjcppEmployees;
    g_pWsjcppEmployees = nullptr;
  }

  if (g_pWsjcppInitEmployees != nullptr) {
    g_pWsjcppInitEmployees->clear();
    delete g_pWsjcppInitEmployees;
    g_pWsjcppInitEmployees = nullptr;
  }

  if (g_pWsjcppInitWith != nullptr) {
    g_pWsjcppInitWith->clear();
    delete g_pWsjcppInitWith;
    g_pWsjcppInitWith = nullptr;
  }
}

void WsjcppEmployees::addEmploy(const std::string &sName, WsjcppEmployBase *pEmploy) {
  WsjcppEmployees::initGlobalVariables();
  if (g_pWsjcppEmployees->find(sName) != g_pWsjcppEmployees->end()) {
    WsjcppEmployees::logThrowError("WsjcppEmployees::addEmploy", "Already registered '" + sName + "'");
  } else {
    g_pWsjcppEmployees->insert(std::pair<std::string, WsjcppEmployBase *>(sName, pEmploy));
    // WsjcppEmployees::logInfo(sName, "Registered");
  }
}

bool WsjcppEmployees::init(const std::vector<std::string> &vStart, bool bSilent) {
  WsjcppEmployees::initGlobalVariables();
  std::string TAG = "WsjcppEmployees::init";

  for (unsigned int i = 0; i < vStart.size(); i++) {
    g_pWsjcppInitEmployees->push_back(vStart[i]);
    g_pWsjcppInitWith->push_back(vStart[i]);
    if (!bSilent) {
      WsjcppEmployees::logInfo(TAG, "with " + vStart[i]);
    }
  }

  bool bRepeat = true;
  while (bRepeat) {
    bRepeat = false;
    std::map<std::string, WsjcppEmployBase *>::iterator it = g_pWsjcppEmployees->begin();
    for (; it != g_pWsjcppEmployees->end(); ++it) {
      std::string sEmployName = it->first;
      WsjcppEmployBase *pEmploy = it->second;

      if (std::find(g_pWsjcppInitEmployees->begin(), g_pWsjcppInitEmployees->end(), sEmployName) != g_pWsjcppInitEmployees->end()) {
        continue;
      }

      unsigned int nRequireLoaded = 0;
      for (unsigned int i = 0; i < pEmploy->loadAfter().size(); i++) {
        std::string sRequireEmploy = pEmploy->loadAfter()[i];
        if (std::find(g_pWsjcppInitEmployees->begin(), g_pWsjcppInitEmployees->end(), sRequireEmploy) != g_pWsjcppInitEmployees->end()) {
          nRequireLoaded++;
        }
      }
      if (pEmploy->loadAfter().size() == nRequireLoaded) {
        if (!pEmploy->init(sEmployName, bSilent)) {
          WsjcppEmployees::logError(TAG, sEmployName + " ... INIT_FAIL");
          return false;
        }
        g_pWsjcppInitEmployees->push_back(sEmployName);
        bRepeat = true;
        if (!bSilent) {
          WsjcppEmployees::logOk(TAG, sEmployName + " ... INIT_OK");
        }
      }
    }
  }
  return true;
}

bool WsjcppEmployees::deinit(bool bSilent) {
  const std::string TAG = "WsjcppEmployees::deinit";
  if (g_pWsjcppInitEmployees == nullptr || g_pWsjcppInitWith == nullptr || g_pWsjcppEmployees == nullptr) {
    WsjcppEmployees::logError(TAG, "You must call WsjcppEmployees::init before deinit");
    return false;
  }

  int initializedCount = g_pWsjcppInitEmployees->size();
  for (int i = initializedCount - 1; i >= 0; i--) {
    std::string sEmployName = g_pWsjcppInitEmployees->at(i);
    if (std::find(g_pWsjcppInitWith->begin(), g_pWsjcppInitWith->end(), sEmployName) != g_pWsjcppInitWith->end()) {
      if (!bSilent) {
        WsjcppEmployees::logInfo(TAG, sEmployName + " ... SKIP_INIT_WITH");
      }
      continue;
    }

    std::map<std::string, WsjcppEmployBase *>::iterator it;
    it = g_pWsjcppEmployees->find(sEmployName);
    if (it == g_pWsjcppEmployees->end()) {
      WsjcppEmployees::logError(TAG, sEmployName + " ... DEINIT_NOT_FOUND");
      return false;
    }
    WsjcppEmployBase *pEmploy = it->second;
    if (pEmploy->deinit(sEmployName, bSilent)) {
      if (!bSilent) {
        WsjcppEmployees::logOk(TAG, sEmployName + " ... DEINIT_OK");
      }
    } else {
      WsjcppEmployees::logError(TAG, sEmployName + " ... DEINIT_FAIL");
      return false;
    }
  };

  g_pWsjcppInitEmployees->clear();
  delete g_pWsjcppInitEmployees;
  g_pWsjcppInitEmployees = nullptr;

  g_pWsjcppInitWith->clear();
  delete g_pWsjcppInitWith;
  g_pWsjcppInitWith = nullptr;
  return true;
}

void WsjcppEmployees::recursiveTestDependencies(const std::vector<std::string> &vNames) {
  std::vector<std::string> v = vNames;
  std::string sEmployName = v[v.size() - 1];
  WsjcppEmployBase *pEmploy = nullptr;

  std::map<std::string, WsjcppEmployBase *>::iterator it;
  it = g_pWsjcppEmployees->find(sEmployName);
  if (it == g_pWsjcppEmployees->end()) {
    // WsjcppEmployees::logThrowError("WsjcppEmployees::recursiveTestDependencies", "Not found employ '" + sEmployName + "'");
    return;
  }
  pEmploy = g_pWsjcppEmployees->at(sEmployName);
  std::vector<std::string> vLoadAfter = pEmploy->loadAfter();
  for (int la = 0; la < vLoadAfter.size(); la++) {
    for (int i = 0; i < v.size(); i++) {
      if (v[i] == vLoadAfter[la]) {
        std::string list_deps = "";
        for (unsigned int li = 0; li < v.size(); ++li) {
          list_deps += v[li] + " -> ";
        }
        list_deps += vLoadAfter[la];
        WsjcppEmployees::logThrowError(
          "WsjcppEmployees::recursiveTestDependencies",
          "Cycle dependency: " + list_deps
        );
        return;
      }
    }
    v.push_back(vLoadAfter[la]);
    recursiveTestDependencies(v);
    v.pop_back();
  }
}

void WsjcppEmployees::logInfo(const std::string &tag, const std::string &message) {
  if (g_wsjcppEmployeesLogger) {
    g_wsjcppEmployeesLogger->info(tag, message);
  }
}

void WsjcppEmployees::logWarning(const std::string &tag, const std::string &message) {
  if (g_wsjcppEmployeesLogger) {
    g_wsjcppEmployeesLogger->warn(tag, message);
  }
}

void WsjcppEmployees::logOk(const std::string &tag, const std::string &message) {
  if (g_wsjcppEmployeesLogger) {
    g_wsjcppEmployeesLogger->ok(tag, message);
  }
}

void WsjcppEmployees::logError(const std::string &tag, const std::string &message) {
  if (g_wsjcppEmployeesLogger) {
    g_wsjcppEmployeesLogger->err(tag, message);
  }
}

void WsjcppEmployees::logThrowError(const std::string &tag, const std::string &message) {
  if (g_wsjcppEmployeesLogger) {
    g_wsjcppEmployeesLogger->throw_err(tag, message);
  }
}

// ---------------------------------------------------------------------
// WsjcppEmployBase

WsjcppEmployBase::WsjcppEmployBase(const std::vector<std::string> &vNames, const std::vector<std::string> &vAfter) {
  TAG = vNames[0];
  m_vNames = vNames;

  for (unsigned int i = 0; i < vAfter.size(); i++) {
    m_vLoadAfter.push_back(vAfter[i]);
  }
  for (int i = 0; i < m_vNames.size(); i++) {
    WsjcppEmployees::addEmploy(m_vNames[i], this);
  }
  WsjcppEmployees::recursiveTestDependencies(m_vNames);
}

WsjcppEmployBase::~WsjcppEmployBase() {
  // nothing
}

const std::vector<std::string> &WsjcppEmployBase::loadAfter() { return m_vLoadAfter; }

// ---------------------------------------------------------------------
// WsjcppEmployRuntimeGlobalCache

REGISTRY_WSJCPP_EMPLOY(WsjcppEmployRuntimeGlobalCache)

WsjcppEmployRuntimeGlobalCache::WsjcppEmployRuntimeGlobalCache()
  : WsjcppEmployBase({WsjcppEmployRuntimeGlobalCache::name()}, {}) {

  TAG = WsjcppEmployRuntimeGlobalCache::name();
}

bool WsjcppEmployRuntimeGlobalCache::init(const std::string &sName, bool bSilent) {
  // checking settings
  if (!bSilent) {
    WsjcppEmployees::logInfo(TAG, "init");
  }
  return true;
}

bool WsjcppEmployRuntimeGlobalCache::deinit(const std::string &sName, bool bSilent) {
  // checking settings
  if (!bSilent) {
    WsjcppEmployees::logInfo(TAG, "deinit");
  }
  m_sStringMap.clear();
  return true;
}

void WsjcppEmployRuntimeGlobalCache::set(const std::string &sName, const std::string &sValue) {
  m_sStringMap[sName] = sValue;
}

bool WsjcppEmployRuntimeGlobalCache::has(const std::string &sName) {
  return m_sStringMap.find(sName) != m_sStringMap.end();
}

std::string WsjcppEmployRuntimeGlobalCache::get(const std::string &sName) {
  if (m_sStringMap.find(sName) != m_sStringMap.end()) {
    return m_sStringMap[sName];
  }
  return "";
}
