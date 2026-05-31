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

#include <wsjcpp_core.h>
#include "ctf01d/objects/ctf01d_service_checker_thread.h"
#include "ctf01d/employees/employ_config.h"
#include "ctf01d/employees/employ_web_server.h"

std::vector<std::string> argumentsToVector(int argc, const char* argv[]) {
  std::vector<std::string> ret;
  for (int i = 0; i < argc; i++) {
    ret.push_back(std::string(argv[i]));
  }
  return ret;
}

std::string tryResolveRelativePath(const std::string &path) {
  std::string ret = path;
  if (ret.size() > 0 && ret[0] != '/') {
      ret = WsjcppCore::getCurrentDirectory() + "/" + ret;
  }
  return wsjcpp::normalizeFilePath(ret);
}

bool findWorkDir(std::vector<std::string> &arguments, std::string &workDir) {
  bool found = false;

  // try find in a program arguments
  for (int i = 0; i < arguments.size(); i++) {
    std::string arg = arguments[i];
    if (arg == "--work-dir" || arg == "-work-dir" || arg == "-w" || arg == "-wd") {
      if (i + 1 < arguments.size()) {
        found = true;
        workDir = tryResolveRelativePath(arguments[i + 1]);
        arguments.erase(arguments.begin() + i, arguments.begin() + i + 2);
        break;
      }
    }
  }

  // Anyway, try read from environment. And override from args.
  if (WsjcppCore::getEnv("CTF01D_WORKDIR", workDir)) {
    std::cout << "Working directory get from environment variable CTF01D_WORKDIR: " << workDir << std::endl;
    return true;
  }

  // try find by default paths
  if (!found) {
    std::vector<std::string> vPossibleFolders = {
      tryResolveRelativePath("./"),
      tryResolveRelativePath("./data_sample/"),
      tryResolveRelativePath("/root/data/")
    };

    for (int i = 0; i < vPossibleFolders.size(); i++) {
      std::string sWorkDir = vPossibleFolders[i];
      if (WsjcppCore::fileExists(sWorkDir + "/config.yml")) {
        found = true;
        workDir = sWorkDir;
        break;
      }
    }
    if (found) {
      std::cout << "Automatically detected working directory: " << workDir << std::endl;
    }
  }
  return found;
}

bool tryFindSingleArgument(const std::vector<std::string> &argNames, std::vector<std::string> &arguments) {
  for (int i = 0; i < arguments.size(); i++) {
    std::string arg = arguments[i];
    if (std::find(argNames.begin(), argNames.end(), arg) != argNames.end()) {
      arguments.erase(arguments.begin() + i);
      return true;
    }
  }
  return false;
}

void printHelp(const std::string &programName) {
  std::cout
    << std::endl
    << "Usage: " << programName << " [OPTIONS] COMMAND" << std::endl
    << std::endl
    << "Jury System for ctf-attack-defense" << std::endl
    << std::endl
    << "  example: 'ctf01d -w ./data_test start'" << std::endl
    << std::endl
    << "OPTIONS:" << std::endl
    << "  --work-dir, -work-dir, -wd, -w path         Custom workspace folder with configs, logging, " << std::endl
    << "                                         checker scripts and etc. (env: CTF01D_WORKDIR)" << std::endl
    << "  --version, -version, -v, version       Print version and exit" << std::endl
    << "  --help, -help, help, -h                Print help and exit" << std::endl
    << std::endl
    << "COMMANDS:" << std::endl
    << "  web-test                               Start alone http server for test" << std::endl
    << "  start                                  Start ctf01d attack-defense jury system." << std::endl
    << std::endl
  ;
}

int main(int argc, const char* argv[]) {
  std::string TAG = "MAIN";
  std::string appName = std::string(WSJCPP_APP_NAME);
  std::string appVersion = std::string(WSJCPP_APP_VERSION);

  // disable log in first
  WsjcppLog::setEnableLogFile(false);
  WsjcppLog::setPrefixLogFile("ctf01d");

  // parse arguments
  std::vector<std::string> arguments = argumentsToVector(argc, argv);
  std::string programName = arguments[0];
  arguments.erase(arguments.begin());

  if (tryFindSingleArgument({"--version", "-version", "version", "-v"}, arguments)) {
    std::cout << appName << " " << appVersion << std::endl;
    return 0;
  }

  if (tryFindSingleArgument({"--help", "-help", "help", "-h"}, arguments)) {
    printHelp(programName);
    return -1;
  }

  std::string sWorkDir;
  if (!findWorkDir(arguments, sWorkDir)) {
    std::cout << "Working directory not found: " << sWorkDir << std::endl;
    return -1;
  }
  std::cout << "WorkDir: " << sWorkDir << std::endl;
  EmployConfig *pConfig = findWsjcppEmploy<EmployConfig>();
  pConfig->setWorkDir(sWorkDir);

  if (arguments.size() == 0) {
    std::cout << "Not found command. Please run '" << programName << " help'" << std::endl;
    return -1;
  }

  if (arguments.size() != 1) {
    std::cout << "Unknown arguments. Please run '" << programName << " help'" << std::endl;
    return -1;
  }

  std::string command = arguments[0];

  if (command == "web-test") {
    WsjcppLog::info(TAG, "Web Test...");
    if (!WsjcppEmployees::init({})) {
        WsjcppLog::err(TAG, "Failed.");
        return -1;
    }
    return findWsjcppEmploy<EmployWebServer>()->start();
  }

  if (command == "start") {
    WsjcppLog::info(TAG, "Starting...");
    if (!WsjcppEmployees::init({})) {
      WsjcppLog::err(TAG, "Start failed on step init configs.");
      return -1;
    }

    // signal( SIGINT, quitApp );
    // signal( SIGTERM, quitApp );

    EmployConfig *pEmployConfig = findWsjcppEmploy<EmployConfig>();

    // TODO move to hot reload and EmployScoreboard::init
    WsjcppLog::info(TAG, "Restoring states from storage...");
    pEmployConfig->scoreboard()->initStateFromStorage();
    WsjcppLog::ok(TAG, "Restored state from storage.");
    std::vector<ctf01d::service_checker_thread *> vThreads;
    WsjcppLog::info(TAG, "Starting threads...");
    for (unsigned int iservice = 0; iservice < pEmployConfig->servicesConf().size(); iservice++) {
      for (unsigned int i_team = 0; i_team < pEmployConfig->teamsConf().size(); i_team++) {
        Ctf01dTeamDef teamConf = pEmployConfig->teamsConf()[i_team];
        ctf01d::service_def serviceConf = pEmployConfig->servicesConf()[iservice];

        // reset status to down
        pEmployConfig->scoreboard()->setServiceStatus(teamConf.getId(), serviceConf.id(), Ctf01dServiceStatusCell::SERVICE_DOWN);
        // pConfig->scoreboard()->setTeamTries();

        ctf01d::service_checker_thread *thr = new ctf01d::service_checker_thread(teamConf, serviceConf);
        thr->start();
        vThreads.push_back(thr);
      }
    }
    WsjcppLog::info(TAG, std::to_string(vThreads.size()) + " threads started");
    return findWsjcppEmploy<EmployWebServer>()->start();
  }

  std::cout << "Unknown command '" << command << "'. Please run '" << programName << " help'" << std::endl;
  return -1;
}
