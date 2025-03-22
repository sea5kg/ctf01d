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

#include "argument_processor_start.h"
#include <wsjcpp_core.h>
#include <employ_config.h>
#include <ctf01d_http_server.h>
#include "WebSocketServer.h"  // libhv

// ---------------------------------------------------------------------
// ArgumentProcessorStart

ArgumentProcessorStart::ArgumentProcessorStart()
: WsjcppArgumentProcessor({"start"}, "TODO short description", "TODO long description") {
    TAG = "ArgumentProcessorStart";
    // registrySingleArgument("--single", "What exactly do this single param?");
    // registryParameterArgument("-param", "N", "What need this param?");
    registryExample("ctf01d -work-dir ./data_test -db-host localhost start");
}

// ---------------------------------------------------------------------

bool ArgumentProcessorStart::applySingleArgument(const std::string &sProgramName, const std::string &sArgumentName) {
    WsjcppLog::err(TAG, "Not implemented");
    return false;
}

// ---------------------------------------------------------------------

bool ArgumentProcessorStart::applyParameterArgument(
    const std::string &sProgramName,
    const std::string &sArgumentName,
    const std::string &sValue
) {
    WsjcppLog::err(TAG, "Not implemented");
    return false;
}

// ---------------------------------------------------------------------

/*void quitApp(int signum) {
    std::cout << std::endl << "Terminating server by signal " << signum << std::endl;
    g_httpServer.stop();
    exit(1);
}*/

int ArgumentProcessorStart::exec(const std::vector<std::string> &vRoutes, const std::vector<std::string> &vSubParams) {
    WsjcppLog::info(TAG, "Starting...");
    if (!WsjcppEmployees::init({})) {
        WsjcppLog::err(TAG, "Start failed on step init configs.");
        return -1;
    }

    // signal( SIGINT, quitApp );
    // signal( SIGTERM, quitApp );

    EmployConfig *pEmployConfig = findWsjcppEmploy<EmployConfig>();

    // TODO move to EmployScoreboard::init
    pEmployConfig->scoreboard()->initStateFromStorage();

    for (unsigned int iservice = 0; iservice < pEmployConfig->servicesConf().size(); iservice++) {
        for (unsigned int iteam = 0; iteam < pEmployConfig->teamsConf().size(); iteam++) {
            Ctf01dTeamDef teamConf = pEmployConfig->teamsConf()[iteam];
            Ctf01dServiceDef serviceConf = pEmployConfig->servicesConf()[iservice];

            // reset status to down
            pEmployConfig->scoreboard()->setServiceStatus(teamConf.getId(), serviceConf.id(), ServiceStatusCell::SERVICE_DOWN);
            // pConfig->scoreboard()->setTeamTries();

            ServiceCheckerThread *thr = new ServiceCheckerThread(teamConf, serviceConf);
            thr->start();
            m_vThreads.push_back(thr);
        }
    }

    WsjcppLog::ok(TAG, "Starting scoreboard on http://localhost:" + std::to_string(pEmployConfig->scoreboardPort()) + "/");

    Ctf01dHttpServer httpServer;
    hv::HttpService *pRouter = httpServer.getService();
    hv::HttpServer server(pRouter);
    server.setPort(pEmployConfig->scoreboardPort());
    server.setThreadNum(4);
    server.run();

    // TODO: stop all threads

    /*while(1) {
        Log::info(TAG, "wait 2 minutes");
        std::this_thread::sleep_for(std::chrono::minutes(2));
        Log::info(TAG, "wait ended");
    }*/

    return -1;
}

