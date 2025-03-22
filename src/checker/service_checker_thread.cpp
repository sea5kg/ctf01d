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

#include "service_checker_thread.h"
#include <unistd.h>

#include <dorunchecker.h>

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

int ServiceCheckerThread::CHECKER_CODE_UP = 101;
int ServiceCheckerThread::CHECKER_CODE_CORRUPT = 102;
int ServiceCheckerThread::CHECKER_CODE_MUMBLE = 103;
int ServiceCheckerThread::CHECKER_CODE_DOWN = 104;
int ServiceCheckerThread::CHECKER_CODE_SHIT = 400;

// ---------------------------------------------------------------------

ServiceCheckerThread::ServiceCheckerThread(
    const Ctf01dTeamDef &teamConf,
    const Ctf01dServiceDef &service
) {
    m_pConfig = findWsjcppEmploy<EmployConfig>();
    m_pDatabase = findWsjcppEmploy<EmployDatabase>();
    m_teamConf = teamConf;
    m_serviceConf = service;
    m_pEmployFlags = findWsjcppEmploy<EmployFlags>();

    TAG = "Checker: " + m_teamConf.getId() + std::string( 15 - m_teamConf.getId().length(), ' ')
         + m_serviceConf.id() + " ";
    WsjcppLog::info(TAG, "Created thread");
}

// ----------------------------------------------------------------------
// newServiceCheckerThread

void* newServiceCheckerThread(void *arg) {
	// Log::info("newRequest", "");
	ServiceCheckerThread *pServerThread = (ServiceCheckerThread *)arg;
	pthread_detach(pthread_self());
	pServerThread->run();
	return 0;
}

// ----------------------------------------------------------------------

void ServiceCheckerThread::start() {
    pthread_create(&m_checkerThread, NULL, &newServiceCheckerThread, (void *)this);
}

// ---------------------------------------------------------------------

int ServiceCheckerThread::runChecker(Ctf01dFlag &flag, const std::string &sCommand) {
    if (sCommand != "put" &&  sCommand != "check") {
        WsjcppLog::err(TAG, "runChecker - sCommand must be 'put' or 'check' ");
        return ServiceCheckerThread::CHECKER_CODE_SHIT;
    }

    // Used code from here
    // https://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix

    std::string sShellCommand = m_serviceConf.scriptPath()
        + " " + m_teamConf.ipAddress()
        + " " + sCommand
        + " " + flag.getId()
        + " " + flag.getValue();

    WsjcppLog::info(TAG, "Start script " + sShellCommand);

    DoRunChecker process(
        m_serviceConf.scriptDir(),
        m_serviceConf.scriptPath(),
        m_teamConf.ipAddress(),
        sCommand, flag.getId(),
        flag.getValue()
    );
    process.start(m_serviceConf.scriptWaitInSec()*1000);

    if (process.isTimeout()) {
        WsjcppLog::err(TAG, "ErrorTimeout on run script service: " + process.outputString());
        return ServiceCheckerThread::CHECKER_CODE_MUMBLE;
    }

    if (process.hasError()) {
        WsjcppLog::err(TAG, "Checker is shit");
        WsjcppLog::err(TAG, "Error on run script service: " + process.outputString());
        return ServiceCheckerThread::CHECKER_CODE_SHIT;
    }

    int nExitCode = process.exitCode();
    if (nExitCode == ServiceCheckerThread::CHECKER_CODE_MUMBLE) {
        WsjcppLog::err(TAG, "Checker says that serivice is mumble...\nLOG:\n"
            "\n" + process.outputString() + "\n\n");
    }

    if (nExitCode == ServiceCheckerThread::CHECKER_CODE_CORRUPT) {
        WsjcppLog::err(TAG, "Checker says that serivice is corrupt... \nLOG:\n"
            "\n" + process.outputString() + "\n\n");
    }

    if (nExitCode != ServiceCheckerThread::CHECKER_CODE_UP
        && nExitCode != ServiceCheckerThread::CHECKER_CODE_MUMBLE
        && nExitCode != ServiceCheckerThread::CHECKER_CODE_CORRUPT
        && nExitCode != ServiceCheckerThread::CHECKER_CODE_DOWN) {
        WsjcppLog::err(TAG, "Wrong checker exit code...\n"
            "\n" + process.outputString());
        return ServiceCheckerThread::CHECKER_CODE_SHIT;
    }

    return nExitCode;
}

// ---------------------------------------------------------------------

void ServiceCheckerThread::run() {
    // TODO: BUG: so here can be problem with mysql connection after 7-8 hours (terminate connection on MySQL side)
    // SOLUTION migrate to PostgreSQL

    // TODO check if game ended

    WsjcppLog::info(TAG, "Starting thread...");
    /*if (QString::fromStdString(m_teamConf.ipAddress()).isEmpty()) {
        WsjcppLog::err(TAG, "User IP Address is empty!!!");
        return;
    }*/

    std::string sScriptPath = m_serviceConf.scriptPath();
    /*
    // already checked on start
    if (!Wsjcpp::fileExists(sScriptPath)) {
        WsjcppLog::err(TAG, "FAIL: Script Path to checker not found '" + sScriptPath + "'");
        // TODO shit status
        return;
    }*/
    int nGameStartUTCInSec = m_pConfig->gameStartUTCInSec();

    while(1) {
        int nCurrentTime = WsjcppCore::getCurrentTimeInSeconds();
        if (
            m_pConfig->gameHasCoffeeBreak()
            && nCurrentTime > m_pConfig->gameCoffeeBreakStartUTCInSec()
            && nCurrentTime < m_pConfig->gameCoffeeBreakEndUTCInSec()
        ) {
            WsjcppLog::info(TAG, "Game on coffeebreak");
            m_pConfig->scoreboard()->setServiceStatus(m_teamConf.getId(), m_serviceConf.id(), ServiceStatusCell::SERVICE_COFFEEBREAK);
            return;
        }

        if (nCurrentTime > m_pConfig->gameEndUTCInSec()) {
            WsjcppLog::warn(TAG, "Game ended (current time: " + std::to_string(nCurrentTime) + ")");
            return;
        };

        if (nCurrentTime < nGameStartUTCInSec) {
            WsjcppLog::warn(TAG, "Game started after: " + std::to_string(nGameStartUTCInSec - nCurrentTime) + " seconds");
            m_pConfig->scoreboard()->setServiceStatus(m_teamConf.getId(), m_serviceConf.id(), ServiceStatusCell::SERVICE_WAIT);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        };

        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();

        // If there is more time left before the end of the game than the life of the flag,
        // then we establish a flag
        if (nCurrentTime < (m_pConfig->gameEndUTCInSec() - m_pConfig->flagTimeliveInMin()*60)) {
            Ctf01dFlag flag;
            flag.generateRandomFlag(
                m_pConfig->flagTimeliveInMin(),
                m_teamConf.getId(),
                m_serviceConf.id(),
                nGameStartUTCInSec
            );

            int nExitCode = this->runChecker(flag, "put");

            // WsjcppLog::ok(TAG, " runChecker: " + std::to_string(nExitCode));

            if (nExitCode == ServiceCheckerThread::CHECKER_CODE_UP) {
                // >>>>>>>>>>> service is UP <<<<<<<<<<<<<<
                WsjcppLog::ok(TAG, " => service is up");
                m_pConfig->scoreboard()->incrementFlagsPuttedAndServiceUp(flag);
            } else if (nExitCode == ServiceCheckerThread::CHECKER_CODE_CORRUPT) {
                // >>>>>>>>>>> service is CORRUPT <<<<<<<<<<<<<<
                WsjcppLog::warn(TAG, " => service is corrupt");
                m_pConfig->scoreboard()->insertFlagPutFail(flag, ServiceStatusCell::SERVICE_CORRUPT, "corrupt");
            } else if (nExitCode == ServiceCheckerThread::CHECKER_CODE_MUMBLE) {
                // >>>>>>>>>>> service is MUMBLE <<<<<<<<<<<<<<
                WsjcppLog::warn(TAG, " => service is mumble");
                WsjcppLog::warn(TAG, "exit_code = " + std::to_string(nExitCode));
                m_pConfig->scoreboard()->insertFlagPutFail(flag, ServiceStatusCell::SERVICE_MUMBLE, "mumble");
            } else if (nExitCode == ServiceCheckerThread::CHECKER_CODE_DOWN) {
                // >>>>>>>>>>> service is DOWN <<<<<<<<<<<<<<
                m_pConfig->scoreboard()->insertFlagPutFail(flag, ServiceStatusCell::SERVICE_DOWN, "down");
                WsjcppLog::warn(TAG, " => service is down");
            } else if (nExitCode == ServiceCheckerThread::CHECKER_CODE_SHIT) {
                // >>>>>>>>>>> checker is SHIT <<<<<<<<<<<<<<
                m_pConfig->scoreboard()->insertFlagPutFail(flag, ServiceStatusCell::SERVICE_SHIT, "shit");
                WsjcppLog::err(TAG, " => checker is shit");
            } else {
                m_pConfig->scoreboard()->insertFlagPutFail(flag, ServiceStatusCell::SERVICE_SHIT, "internal_error");
                WsjcppLog::err(TAG, " => runChecker - wrong code return");
            }
        } else {
            WsjcppLog::info(TAG, "Game ended after: " + std::to_string(m_pConfig->gameEndUTCInSec() - nCurrentTime) + " sec");
            // check some service status or just update to UP (Ha-Ha I'm the real evil!)
        }

        std::vector<Ctf01dFlag> vEndedFlags = m_pConfig->scoreboard()->outdatedFlagsLive(m_teamConf.getId(), m_serviceConf.id());

        for (unsigned int i = 0; i < vEndedFlags.size(); i++) {
            Ctf01dFlag outdatedFlag = vEndedFlags[i];
            m_pConfig->scoreboard()->removeFlagLive(outdatedFlag);

            // if (outdatedFlag.teamStole() != "") {
            //     continue;
            // } else {
            // nobody stole outdatedFlag
            int nCheckExitCode = this->runChecker(outdatedFlag, "check");
            if (nCheckExitCode != ServiceCheckerThread::CHECKER_CODE_UP) {
                // service is not up
                WsjcppLog::info(TAG, "flag_check_fail " + outdatedFlag.getValue());
                m_pDatabase->insertFlagCheckFail(outdatedFlag, "code_" + std::to_string(nCheckExitCode));
            } else {
                // service is up
                // TODO: only if last time (== flag time live) was up
                if (!m_pDatabase->isSomebodyStole(outdatedFlag)) {
                    m_pConfig->scoreboard()->incrementDefenceScore(outdatedFlag);
                }
            }
            // }
        }
        end = std::chrono::system_clock::now();

        int elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
        int ms_sleep = m_serviceConf.timeSleepBetweenRunScriptsInSec()*1000;
        WsjcppLog::info(TAG, "Elapsed milliseconds: " + std::to_string(elapsed_milliseconds) + "ms");
        std::this_thread::sleep_for(std::chrono::milliseconds(ms_sleep - elapsed_milliseconds));
    }
}
