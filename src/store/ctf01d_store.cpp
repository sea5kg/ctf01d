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

#include "ctf01d_store.h"
#include <iostream>
#include <fstream>
#include "wsjcpp_core.h"
#include "http_downloader.h"
#include "json.hpp"

// ---------------------------------------------------------------------
// Ctf01dStoreTeamInfo

Ctf01dStoreTeamInfo::Ctf01dStoreTeamInfo() {
    TAG = "Ctf01dStoreTeamInfo";
};

bool Ctf01dStoreTeamInfo::load(
    const nlohmann::json &jsonTeamInfo,
    const std::string &sBaseUrl
) {
    m_sUuid = jsonTeamInfo["uuid"];
    m_sId = jsonTeamInfo["id"];
    m_sName = jsonTeamInfo["name"];
    m_sLogo = jsonTeamInfo["logo"];
    m_sLogo = sBaseUrl + m_sLogo;
    return true;
}

const std::string &Ctf01dStoreTeamInfo::getUuid() const {
    return m_sUuid;
}

const std::string &Ctf01dStoreTeamInfo::getId() const {
    return m_sId;
}

const std::string &Ctf01dStoreTeamInfo::getName() const {
    return m_sName;
}

const std::string &Ctf01dStoreTeamInfo::getLogo() const {
    return m_sLogo;
}

bool Ctf01dStoreTeamInfo::contains(std::vector<std::string> vTerms) {
    bool bFound = false;
    int nSize = vTerms.size();
    int nContains = 0;
    for (int i = 0; i < nSize; i++) {
        if (
            findStringIC(m_sName, vTerms[i])
            || findStringIC(m_sId, vTerms[i])
        ) {
            nContains++;
        }
    }
    return nContains == nSize;
}


bool Ctf01dStoreTeamInfo::findStringIC(const std::string & strHaystack, const std::string & strNeedle) {
  auto it = std::search(
    strHaystack.begin(), strHaystack.end(),
    strNeedle.begin(), strNeedle.end(),
    [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
  );
  return (it != strHaystack.end() );
}

// ---------------------------------------------------------------------
// Ctf01dStore

Ctf01dStore::Ctf01dStore() {
    TAG = "ctf01d-store";
    m_sCtf01dStoreCacheDir = ".ctf01d-store-cache";
}

bool Ctf01dStore::init() {
    if (!WsjcppCore::dirExists(m_sCtf01dStoreCacheDir)) {
        WsjcppCore::makeDir(m_sCtf01dStoreCacheDir);
    }

    std::string sAllStoriesUrl = "https://raw.githubusercontent.com/sea-kg/ctf01d/master/ctf01d-store/all-stores.json";
    std::string sPath = m_sCtf01dStoreCacheDir + "/all-stores.json";
    if (!WsjcppCore::fileExists(sPath)) {
        std::cout << "\n Downloading... \n from url " << sAllStoriesUrl << "\n to file " << sPath << std::endl;
        if (!downloadFileOverHttps(sAllStoriesUrl, sPath)) {
            WsjcppLog::err(TAG, "Could not download all-stories.json");
            return false;
        }
    }

    if (!WsjcppCore::fileExists(sPath)) {
        std::cerr << "File '" << sPath << "' not found" << std::endl;
        return false;
    }

    std::ifstream ifs(sPath);
    nlohmann::json jf = nlohmann::json::parse(ifs);
    nlohmann::json jsonAllTeamStories = jf["all-team-stores"];

    for (auto& el : jsonAllTeamStories.items()) {
        nlohmann::json jsonTeamStore = el.value();
        std::string sUuid = jsonTeamStore["uuid"];
        std::string sUrl = jsonTeamStore["url"];
        std::string sUrlbaseUrl = extractBaseUrlPath(sUrl);

        std::string sDescription = jsonTeamStore["description"];
        std::string sTeamStoreFilename = m_sCtf01dStoreCacheDir + "/teams_" + sUuid + ".json";
        if (!WsjcppCore::fileExists(sTeamStoreFilename)) {
            std::cout << "\n Downloading teams list... \n   from url " << sUrl << "\n   to file " << sTeamStoreFilename << std::endl;
            if (!downloadFileOverHttps(sUrl, sTeamStoreFilename)) {
                WsjcppLog::err(TAG, "Could not download " + sTeamStoreFilename);
                WsjcppCore::removeFile(sTeamStoreFilename);
                continue;
            }
        } else {
            std::ifstream ifsTeams(sTeamStoreFilename);
            nlohmann::json jfTeams = nlohmann::json::parse(ifsTeams);
            nlohmann::json jsonTeams = jfTeams["teams"];
            for (auto& team : jsonTeams.items()) {
                Ctf01dStoreTeamInfo info;
                if (!info.load(team.value(), sUrlbaseUrl)) {
                    std::cerr << "Problem with team item" << std::endl;
                } else {
                    m_vTeams.push_back(info);
                }
            }
        }
    }
    return true;
}

std::vector<Ctf01dStoreTeamInfo> Ctf01dStore::searchTeam(std::vector<std::string> vTerms) {
    std::vector<Ctf01dStoreTeamInfo> vRet;
    for (int i = 0; i < m_vTeams.size(); i++) {
        if (m_vTeams[i].contains(vTerms)) {
            vRet.push_back(m_vTeams[i]);
        }
    }
    return vRet;
}

std::string Ctf01dStore::extractBaseUrlPath(const std::string &sUrl) {
    std::vector<std::string> vSplited = WsjcppCore::split(sUrl, "/");
    int nSize = vSplited.size();
    vSplited.pop_back();
    return WsjcppCore::join(vSplited, "/") + "/";
}