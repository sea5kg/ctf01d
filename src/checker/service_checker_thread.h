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

#ifndef SERVICE_CHECKER_THREAD_H
#define SERVICE_CHECKER_THREAD_H

#include <ctf01d_scoreboard.h>
#include <employ_config.h>
#include <employ_flags.h>

// struct 

class ServiceCheckerThread {
	public:
		// enum for checker return code
        static int CHECKER_CODE_UP;
        static int CHECKER_CODE_CORRUPT;
		static int CHECKER_CODE_MUMBLE;
		static int CHECKER_CODE_DOWN;
		static int CHECKER_CODE_SHIT;

		ServiceCheckerThread(
			const Ctf01dTeamDef &teamConf,
			const Ctf01dServiceDef &serviceConf
		);
		void start();
		void run();

	private:
		std::string TAG;
		pthread_t m_checkerThread;
		EmployConfig *m_pConfig;
		EmployDatabase *m_pDatabase; // TODO not must be here
		EmployFlags *m_pEmployFlags;
		Ctf01dTeamDef m_teamConf;
		Ctf01dServiceDef m_serviceConf;

		int runChecker(Ctf01dFlag &flag, const std::string &sCommand);
		// int runChecker(Flag &flag, const std::string &sCommand);
		// void run();
};

#endif // SERVICE_CHECKER_THREAD_H
