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

#include "employ_team_logos.h"
#include <wsjcpp_core.h>
#include <filesystem>
#include <employ_config.h>

// ---------------------------------------------------------------------
// EmployTeamLogos

REGISTRY_WJSCPP_SERVICE_LOCATOR(EmployTeamLogos)

EmployTeamLogos::EmployTeamLogos()
: WsjcppEmployBase(EmployTeamLogos::name(), { EmployConfig::name() }) {
    TAG = EmployTeamLogos::name();
    m_nLastUpdateWriteTimeLogosInSec = WsjcppCore::getCurrentTimeInSeconds();
}

bool EmployTeamLogos::init() {
    WsjcppLog::info(TAG, "init");
    return true;
}

bool EmployTeamLogos::deinit() {
    WsjcppLog::info(TAG, "deinit");
    return true;
}

bool EmployTeamLogos::loadTeamLogo(const std::string &sTeamId, const std::string &sTeamLogoPath) {
    if (!WsjcppCore::fileExists(sTeamLogoPath)) {
        WsjcppLog::err(TAG, "File '" + sTeamLogoPath + "' did not found");
        return false;
    }
    Ctf01dTeamLogo *pTeamLogo = new Ctf01dTeamLogo();
    pTeamLogo->sTeamId = sTeamId;
    pTeamLogo->pBuffer = nullptr;
    pTeamLogo->nBufferSize = 0;
    pTeamLogo->sFilename = WsjcppCore::extractFilename(sTeamLogoPath);
    pTeamLogo->sFilepath = sTeamLogoPath;
    if (!WsjcppCore::readFileToBuffer(sTeamLogoPath, &(pTeamLogo->pBuffer), (pTeamLogo->nBufferSize))) {
        delete pTeamLogo;
        WsjcppLog::throw_err(TAG, "Could not read file '" + sTeamLogoPath + "'");
        return false;
    }
    std::filesystem::file_time_type ftime = std::filesystem::last_write_time(sTeamLogoPath.c_str());
    pTeamLogo->nLastWriteTime = std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count();
    // add only if file found
    m_mapTeamLogos[sTeamId] = pTeamLogo;
    WsjcppLog::info(TAG, "Loaded team logo: " + sTeamLogoPath + " for team " + sTeamId + " (last write time file: " + std::to_string(pTeamLogo->nLastWriteTime) + ")");
    return true;
}

Ctf01dTeamLogo *EmployTeamLogos::findTeamLogo(const std::string &sTeamId) {
    std::map<std::string, Ctf01dTeamLogo *>::iterator it = m_mapTeamLogos.find(sTeamId);
    if (it != m_mapTeamLogos.end()) {
        return it->second;
    }
    return nullptr;
}

bool EmployTeamLogos::updateLastWriteTime() {
    if (WsjcppCore::getCurrentTimeInSeconds() - m_nLastUpdateWriteTimeLogosInSec < 30) {
        return false;
    }
    m_nLastUpdateWriteTimeLogosInSec = WsjcppCore::getCurrentTimeInSeconds();
    WsjcppLog::info(TAG, "updateLastWriteTime for team's logos");
    bool bHasChanges = false;
    std::map<std::string, Ctf01dTeamLogo *>::iterator it = m_mapTeamLogos.begin();
    while (it != m_mapTeamLogos.end()) {
        Ctf01dTeamLogo *pTeamLogo = it->second;
        std::filesystem::file_time_type ftime = std::filesystem::last_write_time(pTeamLogo->sFilepath.c_str());
        long nLastWriteTime = std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count();
        if (nLastWriteTime != pTeamLogo->nLastWriteTime) {
            bHasChanges = true;
            delete pTeamLogo->pBuffer;
            pTeamLogo->pBuffer = nullptr;
            pTeamLogo->nBufferSize = 0;
            WsjcppCore::readFileToBuffer(pTeamLogo->sFilepath, &(pTeamLogo->pBuffer), pTeamLogo->nBufferSize);
            pTeamLogo->nLastWriteTime = nLastWriteTime;
        }
        it++;
    }
    return bHasChanges;
}

void EmployTeamLogos::updateScorebordJson(nlohmann::json &jsonScoreboard) {
    std::map<std::string, Ctf01dTeamLogo *>::iterator it = m_mapTeamLogos.begin();
    while (it != m_mapTeamLogos.end()) {
        Ctf01dTeamLogo *pTeamLogo = it->second;
        jsonScoreboard["scoreboard"][pTeamLogo->sTeamId]["logo_last_updated"] = pTeamLogo->nLastWriteTime;
        it++;
    }
}
