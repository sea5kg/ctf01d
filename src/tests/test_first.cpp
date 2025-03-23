#include <vector>
#include <iostream>
#include <wsjcpp_core.h>
#include <employ_flags.h>

int main() {
    int nTimeFlagLifeInMin = 1;
    std::string sTeamId = "team1";
    std::string sServiceId = "service1";

    int nCurrentTime = WsjcppCore::getCurrentTimeInSeconds();
    std::cout << "nCurrentTime=" << nCurrentTime << std::endl;

    int nGameStartUTCInSec = nCurrentTime - 86400; // Game started 24 hours ago
    std::cout << "nGameStartUTCInSec=" << nGameStartUTCInSec << std::endl;

    Ctf01dFlag flag;
    flag.generateRandomFlag(nTimeFlagLifeInMin, sTeamId, sServiceId, nGameStartUTCInSec);

    if (flag.getTeamId() != sTeamId) {
        std::cerr << "Unexpected team1" << std::endl;
        return 1;
    }

    if (flag.getServiceId() != sServiceId) {
        std::cerr << "Unexpected serviceid" << std::endl;
        return 2;
    }

    long nFlagLifeTimeInMs = flag.getTimeEndInMs() - flag.getTimeStartInMs();

    if (nFlagLifeTimeInMs != nTimeFlagLifeInMin*60*1000) {
        std::cerr << "flag life time 1" << std::endl;
        return 3;
    }

    sTeamId = "team2";
    flag.setTeamId(sTeamId);
    if (flag.getTeamId() != sTeamId) {
        std::cerr << "Unexpected teamid 2" << std::endl;
        return 4;
    }

    sServiceId = "service2";
    flag.setServiceId(sServiceId);
    if (flag.getServiceId() != sServiceId) {
        std::cerr << "Unexpected serviceid 2" << std::endl;
        return 5;
    }

    long nStartTimeInMs = 100567622;
    flag.setTimeStartInMs(nStartTimeInMs);
    if (flag.getTimeStartInMs() != nStartTimeInMs) {
        std::cerr << "start time in ms 2" << std::endl;
        return 6;
    }

    long nEndTimeInMs = 1005667621;
    flag.setTimeEndInMs(nEndTimeInMs);
    if (flag.getTimeEndInMs() != nEndTimeInMs) {
        std::cerr << "end time in ms 2" << std::endl;
        return 7;
    }

    std::string sOldId = flag.getId();
    flag.generateId();
    if (flag.getId() == sOldId) {
        std::cerr << "generateId 2" << std::endl;
        return 8;
    }

    std::string sFlagId = "QWHzYEKuTX";
    flag.setId(sFlagId);
    if (flag.getId() != sFlagId) {
        std::cerr << "flag id 2" << std::endl;
        return 9;
    }

    std::string sOldValue = flag.getValue();
    nStartTimeInMs = WsjcppCore::getCurrentTimeInMilliseconds();
    flag.setTimeStartInMs(nStartTimeInMs);
    flag.generateValue(nGameStartUTCInSec);
    if (flag.getValue() == sOldValue) {
        std::cerr << "generateValue 2" << std::endl;
        return 10;
    }

    std::string sFlagValue = "c01dbbac-bb0f-a8b7-02fe-928800000010";
    flag.setValue(sFlagValue);
    if (flag.getValue() != sFlagValue) {
        std::cerr << "flag value 2" << std::endl;
        return 11;
    }

    Ctf01dFlag flag3;
    flag3.copyFrom(flag);

    if (flag3.getId() != sFlagId) {
        std::cerr << "flag id 3" << std::endl;
        return 12;
    }

    if (flag3.getValue() != sFlagValue) {
        std::cerr << "flag value 3" << std::endl;
        return 13;
    }

    if (flag3.getTeamId() != sTeamId) {
        std::cerr << "flag teamid 3" << std::endl;
        return 14;
    }

    if (flag3.getServiceId() != sServiceId) {
        std::cerr << "serviceid 3" << std::endl;
        return 15;
    }

    if (flag3.getTimeStartInMs() != nStartTimeInMs) {
        std::cerr << "start time in ms 3" << std::endl;
        return 16;
    }

    if (flag3.getTimeEndInMs() != nEndTimeInMs) {
        std::cerr << "end time in ms 3" << std::endl;
        return 17;
    }

    std::cout << "ok" << std::endl;
    return 0;
}