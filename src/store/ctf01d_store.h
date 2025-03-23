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

#include <string>
#include <vector>
#include "json.hpp"

class Ctf01dStoreTeamInfo {
    public:
        Ctf01dStoreTeamInfo();
        bool load(
            const nlohmann::json &jsonTeamInfo, 
            const std::string &sBaseUrl
        );

        const std::string &getUuid() const;
        const std::string &getId() const;
        const std::string &getName() const;
        const std::string &getLogo() const;
        bool contains(std::vector<std::string> vTerms);

    private:
        bool findStringIC(const std::string & strHaystack, const std::string & strNeedle);

        std::string TAG;
        std::string m_sUuid;
        std::string m_sId;
        std::string m_sName;
        std::string m_sLogo;
};

class Ctf01dStore {
    public:
        Ctf01dStore();
        bool init();

        std::vector<Ctf01dStoreTeamInfo> searchTeam(std::vector<std::string> vTerms);
    private:
        std::string TAG;
        std::string extractBaseUrlPath(const std::string &sUrl);

        std::string m_sCtf01dStoreCacheDir;
        std::vector<Ctf01dStoreTeamInfo> m_vTeams;
};