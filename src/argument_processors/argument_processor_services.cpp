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

#include "argument_processor_services.h"
#include <wsjcpp_core.h>
#include <wsjcpp_employees.h>
#include <employ_config.h>

// ---------------------------------------------------------------------
// ArgumentProcessorServices

ArgumentProcessorServices::ArgumentProcessorServices() 
: WsjcppArgumentProcessor({"services"}, "Services in config.yml", "Services in config.yml") {
    TAG = "ArgumentProcessorServices";
    registryProcessor(new ArgumentProcessorServicesList());
}

// ---------------------------------------------------------------------
// ArgumentProcessorServicesList

ArgumentProcessorServicesList::ArgumentProcessorServicesList() 
: WsjcppArgumentProcessor({"list", "ls"}, "List services", "List of services exists in config.yml") {
    TAG = "ArgumentProcessorServicesList";
}

// ---------------------------------------------------------------------

int ArgumentProcessorServicesList::exec(const std::vector<std::string> &vRoutes, const std::vector<std::string> &vSubParams) {
    if (vSubParams.size() != 0) {
        WsjcppLog::err(TAG, "Unexpected parameters");
        return -1;
    }
    
    WsjcppLog::info(TAG, "Starting...");
    if (!WsjcppEmployees::init({})) { // TODO silent
        WsjcppLog::err(TAG, "Failed.");
        return -1;
    }

    EmployConfig *pEmployConfig = findWsjcppEmploy<EmployConfig>();

    // TODO move to EmployScoreboard::init
    const std::vector<Ctf01dServiceDef> &vServices = pEmployConfig->servicesConf();
    std::cout
        << std::endl
        << "Services:"
        << std::endl
    ;
    if (vServices.size() > 0) {
        for (int i = 0; i < vServices.size(); i++) {
            Ctf01dServiceDef service = vServices[i];
            if (service.isEnabled()) {
                std::cout
                    << " - id: " << service.id() << " " << std::endl
                    << "   name: " << service.name() << std::endl
                    << "   script-path: " << service.scriptPath() << std::endl
                    << "   script-dir: " << service.scriptDir() << std::endl
                    << "   script-timeout: " << service.scriptWaitInSec() << std::endl
                    << "   script-round: " << service.timeSleepBetweenRunScriptsInSec() << std::endl
                    << std::endl;
            }
        }
        std::cout << std::endl;
    } else {
        std::cout
            << std::endl
            << "Not found services, for add someone please call 'ctf01d services install servicename'"
            << std::endl
            << std::endl
        ;
    }
    return 0;
}
