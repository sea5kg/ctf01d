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

#include <argument_processor_ctf01d_main.h>
#include <wsjcpp_core.h>
#include <employ_config.h>

int main(int argc, const char* argv[]) {
    std::string TAG = "MAIN";
    std::string appName = std::string(WSJCPP_APP_NAME);
    std::string appVersion = std::string(WSJCPP_APP_VERSION);

    // previous logs in current directory
    if (!WsjcppCore::dirExists(".ctf01d")) {
        WsjcppCore::makeDir(".ctf01d");
    }
    WsjcppLog::setPrefixLogFile("ctf01d");
    WsjcppLog::setLogDirectory(".ctf01d");

    // try find config.yml
    std::vector<std::string> vPossibleFolders = {
        "./",
        "./data_sample/",
        "/root/data/"
    };

    for (int i = 0; i < vPossibleFolders.size(); i++) {
        std::string sWorkDir = vPossibleFolders[i];
        if (sWorkDir[0] != '/') {
            sWorkDir = WsjcppCore::getCurrentDirectory() + "/" + sWorkDir;
        }
        sWorkDir = WsjcppCore::doNormalizePath(sWorkDir);
        if (WsjcppCore::fileExists(sWorkDir + "/config.yml")) {
            std::cout << "Automatically detected workdir: " << sWorkDir << std::endl;
            EmployConfig *pConfig = findWsjcppEmploy<EmployConfig>();
            pConfig->setWorkDir(sWorkDir);
            break;
        }
    }

    // websocket_server_t server;
    // server.service = pRouter;
    // server.port = 12345;
    // // server.ws = pWs;
    // websocket_server_run(&server);

    ArgumentProcessorCtf01dMain *pMain = new ArgumentProcessorCtf01dMain();
    WsjcppArguments prog(argc, argv, pMain);
    int nRet = prog.exec();
    return nRet;
}
