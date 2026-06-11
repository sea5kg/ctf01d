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

#include "ctf01d_service_checker_thread.h"
#include <unistd.h>

#include "ctf01d_do_run_checker.h"

#include <iostream>
#include <sstream>
#include <wsjcpp_core.h>
#include <chrono>
#include <thread>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <wsjcpp_core.h>
#include "ctf01d/utils/ctf01d_logger.h"

namespace ctf01d {

int service_checker_thread::CHECKER_CODE_UP = 101;
int service_checker_thread::CHECKER_CODE_CORRUPT = 102;
int service_checker_thread::CHECKER_CODE_MUMBLE = 103;
int service_checker_thread::CHECKER_CODE_DOWN = 104;
int service_checker_thread::CHECKER_CODE_SHIT = 400;

service_checker_thread::service_checker_thread(
  std::shared_ptr<ctf01d::logger> logger,
  const ctf01d::service_config &service_config,
  const ctf01d::team_config &team_config
) {
  m_logger = logger;
  m_pConfig = findWsjcppEmploy<EmployConfig>();
  m_pDatabase = findWsjcppEmploy<EmployDatabase>();
  m_team_config = team_config;
  m_service_config = service_config;
  m_alive_flags = findWsjcppEmploy<IAliveFlags>();

  TAG = "Checker: " + m_team_config.id() + std::string( 15 - m_team_config.id().length(), ' ')
    + m_service_config.id() + " ";
  m_logger->info(TAG, "Created thread");
}

void* newServiceCheckerThread(void *arg) {
  // Log::info("newRequest", "");
  service_checker_thread *pServerThread = (service_checker_thread *)arg;
  pthread_detach(pthread_self());
  pServerThread->run();
  return 0;
}

void service_checker_thread::start() {
  pthread_create(&m_checkerThread, NULL, &newServiceCheckerThread, (void *)this);
}

void service_checker_thread::log_err(const std::string &message) {
  m_logger->err(TAG, message);
  ctf01d::log::err(TAG, message);
}

int service_checker_thread::runChecker(ctf01d::flag &flag, const std::string &sCommand) {
  std::string err_log_message;
  if (sCommand != "put" &&  sCommand != "check") {
    log_err("runChecker - sCommand must be 'put' or 'check' ");
    return service_checker_thread::CHECKER_CODE_SHIT;
  }

  // Used code from here
  // https://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix

  std::string sShellCommand = m_service_config.script_path()
    + " " + m_team_config.ip_or_host()
    + " " + sCommand
    + " " + flag.getId()
    + " " + flag.getValue();

  m_logger->info(TAG, "Start script " + sShellCommand);

  DoRunChecker process(
    m_service_config.script_dir(),
    m_service_config.script_path(),
    m_team_config.ip_or_host(),
    sCommand, flag.getId(),
    flag.getValue()
  );
  process.start(m_service_config.script_timeout_in_seconds()*1000);

  if (process.isTimeout()) {
    log_err("ErrorTimeout on run script service: " + process.outputString());
    return service_checker_thread::CHECKER_CODE_MUMBLE;
  }

  if (process.hasError()) {
    log_err("Checker is shit");
    log_err("Error on run script service: " + process.outputString());
    return service_checker_thread::CHECKER_CODE_SHIT;
  }

  int nExitCode = process.exitCode();
  if (nExitCode == service_checker_thread::CHECKER_CODE_MUMBLE) {
    log_err("Checker says that service is mumble...\nLOG:\n"
      "\n" + process.outputString() + "\n\n");
  }

  if (nExitCode == service_checker_thread::CHECKER_CODE_CORRUPT) {
    log_err("Checker says that service is corrupt... \nLOG:\n"
      "\n" + process.outputString() + "\n\n");
  }

  if (
    nExitCode != service_checker_thread::CHECKER_CODE_UP
    && nExitCode != service_checker_thread::CHECKER_CODE_MUMBLE
    && nExitCode != service_checker_thread::CHECKER_CODE_CORRUPT
    && nExitCode != service_checker_thread::CHECKER_CODE_DOWN
  ) {
    log_err("Wrong checker exit code...\n"
      "\n" + process.outputString());
    return service_checker_thread::CHECKER_CODE_SHIT;
  }

  return nExitCode;
}

void service_checker_thread::run() {
  // TODO check if game ended

  m_logger->info(TAG, "Starting thread...");

  std::string sScriptPath = m_service_config.script_path();
  /*
  // already checked on start
  if (!Wsjcpp::fileExists(sScriptPath)) {
    log_err(TAG, "FAIL: Script Path to checker not found '" + sScriptPath + "'");
    // TODO shit status
    return;
  }*/
  int nGameStartUTCInSec = m_pConfig->gameStartUTCInSec();

  while (1) {
    int nCurrentTime = WsjcppCore::getCurrentTimeInSeconds();
    if (
      m_pConfig->gameHasCoffeeBreak()
      && nCurrentTime > m_pConfig->gameCoffeeBreakStartUTCInSec()
      && nCurrentTime < m_pConfig->gameCoffeeBreakEndUTCInSec()
    ) {
      m_logger->info(TAG, "Game on coffee break");
      m_pConfig->scoreboard()->set_service_status(m_team_config.id(), m_service_config.id(), Ctf01dServiceStatusCell::SERVICE_COFFEE_BREAK);
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      continue;
    }

    if (nCurrentTime > m_pConfig->gameEndUTCInSec()) {
      m_logger->warn(TAG, "Game ended (current time: " + std::to_string(nCurrentTime) + ")");
      return;
    };

    if (nCurrentTime < nGameStartUTCInSec) {
      m_logger->warn(TAG, "Game started after: " + std::to_string(nGameStartUTCInSec - nCurrentTime) + " seconds");
      m_pConfig->scoreboard()->set_service_status(m_team_config.id(), m_service_config.id(), Ctf01dServiceStatusCell::SERVICE_WAIT);
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      continue;
    };

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    // If there is more time left before the end of the game than the life of the flag,
    // then we establish a flag
    if (nCurrentTime < (m_pConfig->gameEndUTCInSec() - m_pConfig->flagLifetimeInSeconds())) {
      ctf01d::flag flag;
      flag.generateRandomFlag(
        m_pConfig->flagLifetimeInSeconds(),
        m_team_config.id(),
        m_service_config.id(),
        nGameStartUTCInSec
      );

      int nExitCode = this->runChecker(flag, "put");

      // m_logger->ok(TAG, " runChecker: " + std::to_string(nExitCode));

      if (nExitCode == service_checker_thread::CHECKER_CODE_UP) {
        // >>>>>>>>>>> service is UP <<<<<<<<<<<<<<
        m_logger->ok(TAG, " => service is up");
        m_pConfig->scoreboard()->increment_flags_putted_and_service_up(flag);
      } else if (nExitCode == service_checker_thread::CHECKER_CODE_CORRUPT) {
        // >>>>>>>>>>> service is CORRUPT <<<<<<<<<<<<<<
        m_logger->warn(TAG, " => service is corrupt");
        m_pConfig->scoreboard()->insert_flag_put_fail(flag, Ctf01dServiceStatusCell::SERVICE_CORRUPT, "corrupt");
      } else if (nExitCode == service_checker_thread::CHECKER_CODE_MUMBLE) {
        // >>>>>>>>>>> service is MUMBLE <<<<<<<<<<<<<<
        m_logger->warn(TAG, " => service is mumble");
        m_logger->warn(TAG, "exit_code = " + std::to_string(nExitCode));
        m_pConfig->scoreboard()->insert_flag_put_fail(flag, Ctf01dServiceStatusCell::SERVICE_MUMBLE, "mumble");
      } else if (nExitCode == service_checker_thread::CHECKER_CODE_DOWN) {
        // >>>>>>>>>>> service is DOWN <<<<<<<<<<<<<<
        m_pConfig->scoreboard()->insert_flag_put_fail(flag, Ctf01dServiceStatusCell::SERVICE_DOWN, "down");
        m_logger->warn(TAG, " => service is down");
      } else if (nExitCode == service_checker_thread::CHECKER_CODE_SHIT) {
        // >>>>>>>>>>> checker is SHIT <<<<<<<<<<<<<<
        m_pConfig->scoreboard()->insert_flag_put_fail(flag, Ctf01dServiceStatusCell::SERVICE_SHIT, "shit");
        m_logger->err(TAG, " => checker is shit");
      } else {
        m_pConfig->scoreboard()->insert_flag_put_fail(flag, Ctf01dServiceStatusCell::SERVICE_SHIT, "internal_error");
        m_logger->err(TAG, " => runChecker - wrong code return");
      }
    } else {
      m_logger->info(TAG, "Game ended after: " + std::to_string(m_pConfig->gameEndUTCInSec() - nCurrentTime) + " sec");
      // check some service status or just update to UP (Ha-Ha I'm the real evil!)
    }

    std::vector<ctf01d::flag> vEndedFlags = m_alive_flags->outdated_alive_flags(m_team_config.id(), m_service_config.id());

    for (unsigned int i = 0; i < vEndedFlags.size(); i++) {
      ctf01d::flag outdatedFlag = vEndedFlags[i];
      m_alive_flags->remove_alive_flag(outdatedFlag);

      // if (outdatedFlag.teamStole() != "") {
      //     continue;
      // } else {
      // nobody stole outdatedFlag
      int nCheckExitCode = this->runChecker(outdatedFlag, "check");
      if (nCheckExitCode != service_checker_thread::CHECKER_CODE_UP) {
            // service is not up
            m_logger->info(TAG, "flag_check_fail " + outdatedFlag.getValue());
            m_pDatabase->insertFlagCheckFail(outdatedFlag, "code_" + std::to_string(nCheckExitCode));
      } else {
        // service is up
        // TODO: only if last time (== flag time live) was up
        if (!m_pDatabase->isSomebodyStole(outdatedFlag)) {
          m_pConfig->scoreboard()->increment_defense_score(outdatedFlag);
        }
      }
        // }
    }
    end = std::chrono::system_clock::now();

    int elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    int ms_sleep = m_service_config.round_in_seconds()*1000;
    m_logger->info(TAG, "Elapsed milliseconds: " + std::to_string(elapsed_milliseconds) + "ms");
    std::this_thread::sleep_for(std::chrono::milliseconds(ms_sleep - elapsed_milliseconds));
  }
}

} // namespace ctf01d
