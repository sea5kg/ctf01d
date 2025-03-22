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

#ifndef EMPLOY_FLAGS_H
#define EMPLOY_FLAGS_H

#include <wsjcpp_employees.h>
#include <string>
#include <mutex>
#include <fstream>

class Ctf01dFlag {
    public:
        Ctf01dFlag();
        void generateRandomFlag(int nTimeFlagLifeInMin, const std::string &sTeamId, const std::string &sServiceId, int nGameStartUTCInSec);

        void generateId();
        void setId(const std::string &sId);
        std::string getId() const;

        void generateValue(int nGameStartUTCInSec);
        void setValue(const std::string &sValue);
        std::string getValue() const;

        void setTeamId(const std::string &sTeamId);
        const std::string &getTeamId() const;

        void setServiceId(const std::string &sServiceId);
        const std::string &getServiceId() const;

        void setTimeStartInMs(long nTimeStart);
        long getTimeStartInMs() const;

        void setTimeEndInMs(long nTimeEnd);
        long getTimeEndInMs() const;

        void copyFrom(const Ctf01dFlag &flag);

    private:
        std::string m_sId;
        std::string m_sValue;
        std::string m_sTeamId;
        std::string m_sServiceId;
        long m_nTimeStartInMs;
        long m_nTimeEndInMs;
};

class EmployFlags : public WsjcppEmployBase {
    public:
        EmployFlags();
        static std::string name() { return "EmployFlags"; }
        virtual bool init() override;
        virtual bool deinit() override;

    private:
        std::string TAG;
};

#endif // EMPLOY_FLAGS_H
