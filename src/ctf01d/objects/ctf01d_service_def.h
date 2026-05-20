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

#include <string>

class Ctf01dServiceDef {
public:
  Ctf01dServiceDef();

  void setId(const std::string &sServiceId);
  const std::string &id() const;

  void setName(const std::string &sName);
  const std::string &name() const;

  void setScriptPath(const std::string &sScriptPath);
  const std::string &scriptPath() const;

  void setScriptDir(const std::string &sScriptDir);
  const std::string &scriptDir() const;

  void setEnabled(bool bEnabled);
  bool isEnabled() const;

  void setScriptWaitInSec(int nSec);
  int scriptWaitInSec() const;

  void setTimeSleepBetweenRunScriptsInSec(int nSec);
  int timeSleepBetweenRunScriptsInSec() const;

private:
  int m_nNum;
  bool m_bEnabled;
  int m_nScriptWaitInSec;
  int m_nTimeSleepBetweenRunScriptsInSec;
  std::string m_sID;
  std::string m_sName;
  std::string m_sScriptPath;
  std::string m_sScriptDir;
};
