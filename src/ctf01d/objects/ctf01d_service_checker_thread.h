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

#include "ctf01d/objects/ctf01d_scoreboard.h"
#include "ctf01d/employees/employ_config.h"
#include "ctf01d/include/i_alive_flags.h"

namespace ctf01d {

class service_checker_thread {
public:
  // enum for checker return code
  static int CHECKER_CODE_UP;
  static int CHECKER_CODE_CORRUPT;
  static int CHECKER_CODE_MUMBLE;
  static int CHECKER_CODE_DOWN;
  static int CHECKER_CODE_SHIT;
  service_checker_thread(const ctf01d::team_config &teamConf, const ctf01d::service_config &serviceConf);
  void start();
  void run();

private:
  std::string TAG;
  pthread_t m_checkerThread;
  EmployConfig *m_pConfig;
  EmployDatabase *m_pDatabase; // TODO not must be here
  IAliveFlags *m_alive_flags;
  ctf01d::team_config m_teamConf;
  ctf01d::service_config m_serviceConf;

  int runChecker(ctf01d::flag &flag, const std::string &sCommand);
  // int runChecker(Flag &flag, const std::string &sCommand);
  // void run();
};

} // namespace ctf01d
