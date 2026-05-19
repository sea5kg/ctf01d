/*
 * MIT License
 *
 * Copyright (c) 2019-2026 Evgenii Sopov
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
*/

// original source-code: https://github.com/wsjcpp/wsjcpp-core

#ifndef WSJCPP_UNIT_TESTS_H
#define WSJCPP_UNIT_TESTS_H

#include <wsjcpp_core.h>
#include <sstream>

class WsjcppUnitTestBase {
    public:
        WsjcppUnitTestBase(const std::string &sTestName);
        std::string getName();
        void ok(const std::string &sSuccessMessage);
        void fail(const std::string &sFailedMessage);
        bool runTest();

        virtual bool doBeforeTest() = 0;
        virtual void executeTest() = 0;
        virtual bool doAfterTest() = 0;
    protected:
        std::string TAG;

        bool compareD(const std::string &sMark, double nValue, double nExpected);
        template<typename T1, typename T2> bool compare(const std::string &sMark, T1 tGotValue, T2 tExpectedValue) {
            if (tGotValue != tExpectedValue) {
                std::stringstream ss;
                ss << " {mark: " << sMark << "} Expected '" << tExpectedValue << "', but got '" << tGotValue << "'";
                fail(ss.str());
                return false;
            }
            return true;
        }
    private:
        bool m_bTestResult;
        std::string m_sTestName;
};

extern std::vector<WsjcppUnitTestBase*> *g_pWsjcppUnitTests;

class WsjcppUnitTests {
    public:
        static void initGlobalVariables();
        static void addUnitTest(const std::string &sTestName, WsjcppUnitTestBase* pUnitTest);
};

// RegistryCmdHandler
#define REGISTRY_WSJCPP_UNIT_TEST( classname ) \
    static classname * pRegistryWsjcppUnitTest ## classname = new classname(); \


#endif // WSJCPP_UNIT_TESTS_H