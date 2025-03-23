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

#ifndef DORUNCHECKER_H
#define DORUNCHECKER_H

#include <string>
#include <stdio.h>
#include <iostream>
#include <mutex>
#include <deque>

class DoRunChecker {
    public:
        DoRunChecker(
            const std::string &sDir,
            const std::string &sScript,
            const std::string &sIp,
            const std::string &sCommand,
            const std::string &sFlagId,
            const std::string &sFlag
        );
        void start(int nTimeoutMS);
        bool hasError();
        int exitCode();
        bool isTimeout();
        const std::string &outputString();
        void run();

    private:
        std::string TAG;
        std::string m_sDir;
        std::string m_sScript;
        std::string m_sIp;
        std::string m_sCommand;
        std::string m_sFlagId;
        std::string m_sFlag;
        int m_nTimeoutMS;
        pid_t m_nPid;
        pthread_t m_pProcessThread;
        int m_nExitCode;
        bool m_bHasError;
        bool m_bFinished;
        bool m_bFinishedByTimeout;
        std::string m_sOutput;
};


#endif // DORUNCHECKER_H