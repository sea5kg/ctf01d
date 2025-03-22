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

#ifndef EMPLOY_DATABASE_H
#define EMPLOY_DATABASE_H

#include <wsjcpp_employees.h>
#include <json.hpp>
#include <employ_flags.h>
#include <sqlite3.h>

class Ctf01dDatabaseSelectRows {
    public:
        Ctf01dDatabaseSelectRows();
        ~Ctf01dDatabaseSelectRows();
        void setQuery(sqlite3_stmt* pQuery);
        bool next();
        std::string getString(int nColumnNumber);
        long getLong(int nColumnNumber);

    private:
        sqlite3_stmt* m_pQuery;
};


class Ctf01dDatabaseFile {
    public:
        Ctf01dDatabaseFile(const std::string &sFilename, const std::string &sSqlCreateTable);
        ~Ctf01dDatabaseFile();
        bool open();
        bool executeQuery(std::string sSqlInsert);
        int selectSumOrCount(std::string sSqlSelectCount);
        bool selectRows(std::string sSqlSelectRows, Ctf01dDatabaseSelectRows &selectRows);

    private:

        void copyDatabaseToBackup();
        std::mutex m_mutex;

        std::string TAG;
        sqlite3* m_pDatabaseFile;
        std::string m_sFilename;
        std::string m_sFileFullpath;
        std::string m_sBaseFileBackupFullpath;
        std::string m_sSqlCreateTable;
        int m_nLastBackupTime;
};


class EmployDatabase : public WsjcppEmployBase {
    public:
        EmployDatabase();
        static std::string name() { return "EmployDatabase"; }
        virtual bool init() override;
        virtual bool deinit() override;

        // flags_checker_put_states
        void insertToFlagsCheckerPutResult(Ctf01dFlag flag, std::string sResult);
        int numberOfFlagFlagsCheckerPutAllResults(std::string sTeamId, std::string sServiceId);
        int numberOfFlagFlagsCheckerPutSuccessResult(std::string sTeamId, std::string sServiceId);

        void insertFlagAttempt(std::string sTeamId, std::string sFlag);
        int numberOfFlagAttempts(std::string sTeamId);

        void insertToFlagsDefence(Ctf01dFlag flag, int nPoints);
        int numberOfFlagsDefense(std::string sTeamId, std::string sServiceId);
        int sumPointsOfFlagsDefense(std::string sTeamId, std::string sServiceId);
        int numberOfDefenceFlagForService(std::string sServiceId);
        void insertFlagCheckFail(Ctf01dFlag flag, std::string sReason);

        // flags_stollen
        int numberOfFlagsStollen(std::string sTeamId, std::string sServiceId);
        int sumPointsOfFlagsStollen(std::string sTeamId, std::string sServiceId);
        int numberOfStolenFlagsForService(std::string sServiceId);
        std::string getFirstbloodFromStolenFlagsForService(std::string sServiceId);
        void insertToFlagsStolen(Ctf01dFlag flag, std::string sTeamId, int nPoints);
        bool isAlreadyStole(Ctf01dFlag flag, std::string sTeamId);
        bool isSomebodyStole(Ctf01dFlag flag);

        // flags live
        void insertToFlagLive(Ctf01dFlag flag);
        void deleteFlagLive(Ctf01dFlag flag);
        std::vector<Ctf01dFlag> listOfLiveFlags();

    private:

        std::string TAG;
        std::mutex m_mutexCopyDatabaseToBackup;
        Ctf01dDatabaseFile *m_pFlagsAttempts;
        Ctf01dDatabaseFile *m_pFlagsDefense;
        Ctf01dDatabaseFile *m_pFlagsCheckFails;
        Ctf01dDatabaseFile *m_pFlagsStolen;
        Ctf01dDatabaseFile *m_pFlagsLive;
        Ctf01dDatabaseFile *m_pFlagsCheckerPutsResults;
};

#endif // EMPLOY_DATABASE_H
