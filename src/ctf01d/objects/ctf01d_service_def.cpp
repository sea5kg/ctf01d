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

#include "ctf01d_service_def.h"

Ctf01dServiceDef::Ctf01dServiceDef(){
    m_nScriptWaitInSec = 10;
    m_bEnabled = true;
    m_round_in_seconds = 15;
}

void Ctf01dServiceDef::setId(const std::string &sServiceID){
    m_sID = sServiceID;
}

const std::string &Ctf01dServiceDef::id() const {
    return m_sID;
}

void Ctf01dServiceDef::setName(const std::string &sName){
    m_sName = sName;
}

const std::string &Ctf01dServiceDef::name() const {
    return m_sName;
}

void Ctf01dServiceDef::setScriptPath(const std::string &sScriptPath){
    m_sScriptPath = sScriptPath;
}

const std::string &Ctf01dServiceDef::scriptPath() const {
    return m_sScriptPath;
}

void Ctf01dServiceDef::setScriptDir(const std::string &sScriptDir) {
    m_sScriptDir = sScriptDir;
}

const std::string &Ctf01dServiceDef::scriptDir() const {
    return m_sScriptDir;
}

void Ctf01dServiceDef::setEnabled(bool bEnabled){
    m_bEnabled = bEnabled;
}

bool Ctf01dServiceDef::isEnabled() const {
    return m_bEnabled;
}

void Ctf01dServiceDef::setScriptWaitInSec(int nSec){
    m_nScriptWaitInSec = nSec;
    if(m_nScriptWaitInSec < 1){
        m_nScriptWaitInSec = 10;
    }
}

int Ctf01dServiceDef::scriptWaitInSec() const {
    return m_nScriptWaitInSec;
}

void Ctf01dServiceDef::set_round_in_seconds(int nSec){
    m_round_in_seconds = nSec;
    if(m_round_in_seconds < 1){
        m_round_in_seconds = 10;
    }
}

int Ctf01dServiceDef::round_in_seconds() const {
    return m_round_in_seconds;
}
