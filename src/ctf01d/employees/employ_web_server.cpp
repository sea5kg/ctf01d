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

#include "employ_web_server.h"
#include <employ_config.h>
#include <wsjcpp_core.h>
#include <fstream>
#include <cstring>
#include <ctf01d_http_server.h>
#include "WebSocketServer.h"  // libhv

REGISTRY_WSJCPP_EMPLOY(EmployWebServer)

EmployWebServer::EmployWebServer()
: WsjcppEmployBase({ EmployWebServer::name() }, { EmployConfig::name() }) {

}

bool EmployWebServer::init(const std::string &name, bool bSilent) {
  WsjcppLog::info(TAG, "init");
  return true;
}

bool EmployWebServer::deinit(const std::string &name, bool bSilent) {
  WsjcppLog::info(TAG, "deinit");
  return true;
}

int EmployWebServer::start() {

  EmployConfig *pEmployConfig = findWsjcppEmploy<EmployConfig>();

  WsjcppLog::ok(TAG, "Starting scoreboard on http://localhost:" + std::to_string(pEmployConfig->scoreboardPort()) + "/");

  Ctf01dHttpServer httpServer;
  std::shared_ptr<hv::HttpService> pRouter = httpServer.getService();
  hv::HttpServer server(pRouter.get());
  server.setPort(pEmployConfig->scoreboardPort());
  server.setThreadNum(4);
  server.run();

  // TODO: stop all threads

  /*while(1) {
    Log::info(TAG, "wait 2 minutes");
    std::this_thread::sleep_for(std::chrono::minutes(2));
    Log::info(TAG, "wait ended");
  }*/

  return 0;
}


