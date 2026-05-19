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

#include "wsjcpp_unit_tests.h"
#include <cmath>

// ---------------------------------------------------------------------
// WsjcppUnitTestBase

WsjcppUnitTestBase::WsjcppUnitTestBase(const std::string &sTestName) {
    m_sTestName = sTestName;
    TAG = m_sTestName;
    m_bTestResult = true;
    WsjcppUnitTests::addUnitTest(sTestName, this);
}

// ---------------------------------------------------------------------

std::string WsjcppUnitTestBase::getName() {
    return m_sTestName;
}

// ---------------------------------------------------------------------

void WsjcppUnitTestBase::ok(const std::string &sSuccessMessage) {
    // print obly success message
    WsjcppLog::ok(TAG,  sSuccessMessage);
}

// ---------------------------------------------------------------------

void WsjcppUnitTestBase::fail(const std::string &sFailedMessage) {
    WsjcppLog::err(TAG, sFailedMessage);
    m_bTestResult = false;
}

// ---------------------------------------------------------------------

bool WsjcppUnitTestBase::runTest() {
    WsjcppLog::info(TAG, "Start unit-test");
    WsjcppLog::info(TAG, "Do before unit-test");
    if (!doBeforeTest()) {
        fail("Problem with before unit-test");
        return false;
    }
    WsjcppLog::info(TAG,  "Execute unit-test");
    try {
        executeTest();
    } catch(const std::exception& e) {
        fail(e.what());
    } catch(...) {

    }
    if (m_bTestResult) {
        ok("Test passed.");
    } else {
        fail("Test failed.");
    }
    WsjcppLog::info(TAG, "Do after unit-test");
    if (!doAfterTest()) {
        fail("Problem with after unit-test");
    }
    WsjcppLog::info(TAG, "End unit-test");
    return m_bTestResult;
}

// ---------------------------------------------------------------------

bool WsjcppUnitTestBase::compareD(const std::string &sMark, double nValue, double nExpected) {
    if (abs(nValue - nExpected) > std::numeric_limits<double>::epsilon()) {
        fail(" {" + sMark + "} Expected '" + std::to_string(nExpected) + "', but got '" + std::to_string(nValue) + "'");
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------

std::vector<WsjcppUnitTestBase*> *g_pWsjcppUnitTests = nullptr;

void WsjcppUnitTests::initGlobalVariables() {
    if (g_pWsjcppUnitTests == nullptr) {
        // WsjcppLog::info(std::string(), "Create handlers map");
        g_pWsjcppUnitTests = new std::vector<WsjcppUnitTestBase*>();
    }
}

// ---------------------------------------------------------------------

void WsjcppUnitTests::addUnitTest(const std::string &sTestName, WsjcppUnitTestBase* pUnitTest) {
    WsjcppUnitTests::initGlobalVariables();
    bool bFound = false;
    for (int i = 0; i < g_pWsjcppUnitTests->size(); i++) {
        WsjcppUnitTestBase* p = g_pWsjcppUnitTests->at(i);
        if (p->getName() == sTestName) {
            bFound = true;
        }
    }

    if (bFound) {
        WsjcppLog::err(sTestName, "Already registered");
    } else {
        g_pWsjcppUnitTests->push_back(pUnitTest);
        // Log::info(sCmd, "Registered");
    }
}

// ---------------------------------------------------------------------