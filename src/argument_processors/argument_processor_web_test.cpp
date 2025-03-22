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

#include "argument_processor_web_test.h"
#include <wsjcpp_core.h>
#include <employ_config.h>
#include <ctf01d_http_server.h>
#include "WebSocketServer.h" // libhv

// ---------------------------------------------------------------------
// ArgumentProcessorWebTest

ArgumentProcessorWebTest::ArgumentProcessorWebTest()
: WsjcppArgumentProcessor({"web-test"}, "Start alone http server for test", "Start alone http server for test") {
    TAG = "ArgumentProcessorWebTest";
    // registrySingleArgument("--single", "What exactly do this single param?");
    // registryParameterArgument("-param", "N", "What need this param?");
    // registryExample("here example of command");
    // registryProcessor(new ArgumentProcessorOtherProcessor());
}

// ---------------------------------------------------------------------

bool ArgumentProcessorWebTest::applySingleArgument(const std::string &sProgramName, const std::string &sArgumentName) {
    WsjcppLog::err(TAG, "Not implemented");
    return false;
}

// ---------------------------------------------------------------------

bool ArgumentProcessorWebTest::applyParameterArgument(
    const std::string &sProgramName,
    const std::string &sArgumentName,
    const std::string &sValue
) {
    WsjcppLog::err(TAG, "Not implemented");
    return false;
}

// ---------------------------------------------------------------------

int ArgumentProcessorWebTest::exec(const std::vector<std::string> &vRoutes, const std::vector<std::string> &vSubParams) {
    WsjcppLog::info(TAG, "Web Test...");
    if (!WsjcppEmployees::init({})) {
        WsjcppLog::err(TAG, "Failed.");
        return -1;
    }

    EmployConfig *pEmployConfig = findWsjcppEmploy<EmployConfig>();

    WsjcppLog::ok(TAG, "Starting scoreboard on http://localhost:" + std::to_string(pEmployConfig->scoreboardPort()) + "/");

    Ctf01dHttpServer httpServer;
    hv::HttpService *pRouter = httpServer.getService();
    hv::HttpServer server(pRouter);
    server.setPort(pEmployConfig->scoreboardPort());
    server.setThreadNum(4);
    server.run();

    WsjcppLog::err(TAG, "Not implemented");
    return -1;
}

