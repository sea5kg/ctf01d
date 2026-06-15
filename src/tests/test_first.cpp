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

#include <vector>
#include <iostream>
#include <wsjcpp_core.h>
#include "ctf01d/objects/ctf01d_flag.h"

int main() {
    int flag_lifetime_in_seconds = 60;
    std::string sTeamId = "team1";
    std::string sServiceId = "service1";

    int nCurrentTime = WsjcppCore::getCurrentTimeInSeconds();
    std::cout << "nCurrentTime=" << nCurrentTime << std::endl;

    int nGameStartUTCInSec = nCurrentTime - 86400; // Game started 24 hours ago
    std::cout << "nGameStartUTCInSec=" << nGameStartUTCInSec << std::endl;

    ctf01d::flag flag;
    flag.generate_random_flag(flag_lifetime_in_seconds, sTeamId, sServiceId, nGameStartUTCInSec);

    if (flag.team_id() != sTeamId) {
        std::cerr << "Unexpected team1" << std::endl;
        return 1;
    }

    if (flag.service_id() != sServiceId) {
        std::cerr << "Unexpected serviceid" << std::endl;
        return 2;
    }

    long nFlagLifeTimeInMs = flag.time_end_in_milliseconds() - flag.time_start_in_milliseconds();

    if (nFlagLifeTimeInMs != flag_lifetime_in_seconds*1000) {
        std::cerr << "flag life time 1" << std::endl;
        return 3;
    }

    sTeamId = "team2";
    flag.set_team_id(sTeamId);
    if (flag.team_id() != sTeamId) {
        std::cerr << "Unexpected team-id 2" << std::endl;
        return 4;
    }

    sServiceId = "service2";
    flag.set_service_id(sServiceId);
    if (flag.service_id() != sServiceId) {
        std::cerr << "Unexpected serviceid 2" << std::endl;
        return 5;
    }

    long nStartTimeInMs = 100567622;
    flag.set_time_start_in_milliseconds(nStartTimeInMs);
    if (flag.time_start_in_milliseconds() != nStartTimeInMs) {
        std::cerr << "start time in ms 2" << std::endl;
        return 6;
    }

    long nEndTimeInMs = 1005667621;
    flag.set_time_end_in_milliseconds(nEndTimeInMs);
    if (flag.time_end_in_milliseconds() != nEndTimeInMs) {
        std::cerr << "end time in ms 2" << std::endl;
        return 7;
    }

    std::string sOldId = flag.getId();
    flag.generate_id();
    if (flag.getId() == sOldId) {
        std::cerr << "generate_id 2" << std::endl;
        return 8;
    }

    std::string flag_id = "QWHzYEKuTX";
    flag.set_id(flag_id);
    if (flag.getId() != flag_id) {
        std::cerr << "flag id 2" << std::endl;
        return 9;
    }

    std::string sOldValue = flag.value();
    nStartTimeInMs = WsjcppCore::getCurrentTimeInMilliseconds();
    flag.set_time_start_in_milliseconds(nStartTimeInMs);
    flag.generate_value(nGameStartUTCInSec);
    if (flag.value() == sOldValue) {
        std::cerr << "generate_value 2" << std::endl;
        return 10;
    }

    std::string sFlagValue = "c01dbbac-bb0f-a8b7-02fe-928800000010";
    flag.set_value(sFlagValue);
    if (flag.value() != sFlagValue) {
        std::cerr << "flag value 2" << std::endl;
        return 11;
    }

    ctf01d::flag flag3;
    flag3.copy_from(flag);

    if (flag3.getId() != flag_id) {
        std::cerr << "flag id 3" << std::endl;
        return 12;
    }

    if (flag3.value() != sFlagValue) {
        std::cerr << "flag value 3" << std::endl;
        return 13;
    }

    if (flag3.team_id() != sTeamId) {
        std::cerr << "flag team_id 3" << std::endl;
        return 14;
    }

    if (flag3.service_id() != sServiceId) {
        std::cerr << "serviceid 3" << std::endl;
        return 15;
    }

    if (flag3.time_start_in_milliseconds() != nStartTimeInMs) {
        std::cerr << "start time in ms 3" << std::endl;
        return 16;
    }

    if (flag3.time_end_in_milliseconds() != nEndTimeInMs) {
        std::cerr << "end time in ms 3" << std::endl;
        return 17;
    }

    std::cout << "ok" << std::endl;
    return 0;
}