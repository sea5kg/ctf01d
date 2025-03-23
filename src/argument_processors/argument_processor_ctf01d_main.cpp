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

#include "argument_processor_ctf01d_main.h"
#include <wsjcpp_core.h>
#include <employ_config.h>
#include <argument_processor_start.h>
#include <argument_processor_clean.h>
#include <argument_processor_check.h>
#include <argument_processor_version.h>
#include <argument_processor_web_test.h>
#include "argument_processor_teams.h"
#include "argument_processor_services.h"

// ---------------------------------------------------------------------
// ArgumentProcessorCtf01dMain

ArgumentProcessorCtf01dMain::ArgumentProcessorCtf01dMain()
: WsjcppArgumentProcessor({"ctf01d_main"}, "Jury System for ctf-attack-defence", "Jury System for ctf-attack-defence") {
    TAG = "ArgumentProcessorCtf01dMain";
    // registrySingleArgument("--single", "What exactly do this single param?");
    registryParameterArgument("-work-dir", "path", "Custom workspace folder with configs, logging, checker scripts and etc. (env: CTF01D_WORKDIR)");
    registryExample("ctf01d -work-dir ./data_test -db-host localhost start");
    registryProcessor(new ArgumentProcessorVersion());
    registryProcessor(new ArgumentProcessorCheck());
    registryProcessor(new ArgumentProcessorClean());
    registryProcessor(new ArgumentProcessorWebTest());
    registryProcessor(new ArgumentProcessorStart());
    registryProcessor(new ArgumentProcessorTeams());
    registryProcessor(new ArgumentProcessorServices());
}

// ---------------------------------------------------------------------

bool ArgumentProcessorCtf01dMain::applyParameterArgument(
    const std::string &sProgramName,
    const std::string &sArgumentName,
    const std::string &sValue
) {
    EmployConfig *pConfig = findWsjcppEmploy<EmployConfig>();

    if (sArgumentName == "-work-dir") {
        std::string sWorkDir = WsjcppCore::getCurrentDirectory() + sValue;
        sWorkDir = WsjcppCore::doNormalizePath(sWorkDir);
        pConfig->setWorkDir(sWorkDir);
        return true;
    }

    return false;
}