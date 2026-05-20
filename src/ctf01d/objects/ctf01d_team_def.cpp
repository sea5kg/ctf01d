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

#include "ctf01d_team_def.h"

Ctf01dTeamDef::Ctf01dTeamDef() {
    // nothing
}

void Ctf01dTeamDef::setId(const std::string &sTeamId){
    m_sTeamID = sTeamId;
}

const std::string &Ctf01dTeamDef::getId() const {
    return m_sTeamID;
}

void Ctf01dTeamDef::setName(const std::string &sName){
    m_sName = sName;
}

const std::string &Ctf01dTeamDef::getName() const {
    return m_sName;
}

void Ctf01dTeamDef::setIpAddress(const std::string &sIpAddress){
    m_sIpAddress = sIpAddress;
}

const std::string &Ctf01dTeamDef::ipAddress() const {
    return m_sIpAddress;
}

void Ctf01dTeamDef::setActive(bool bActive){
    m_bActive = bActive;
}

bool Ctf01dTeamDef::isActive() const {
    return m_bActive;
}

void Ctf01dTeamDef::setLogo(const std::string &sLogo){
    m_sLogo = sLogo;
}

const std::string &Ctf01dTeamDef::logo() const {
    return m_sLogo;
}

int Ctf01dTeamDef::getLogoLastWriteTime() {
    return m_nLogoLastWriteTime;
}