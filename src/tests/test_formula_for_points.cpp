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

#include <vector>
#include <iostream>
#include <wsjcpp_core.h>
#include <memory>
#include "ctf01d/objects/ctf01d_formulas_for_points.h"

int main() {

    std::shared_ptr<ctf01d::formulas_for_points> formula = std::make_shared<ctf01d::formulas_for_points_ructf>();

    struct LocalDataTestRuCtf {
        LocalDataTestRuCtf(int base_points, int victim_place, int thief_place, int team_count, int expected)
        : base_points(base_points), victim_place(victim_place), thief_place(thief_place), team_count(team_count), expected(expected) {
        }
        std::string to_string() {
            std::string ret = "";
            ret += "base_points=" + std::to_string(base_points);
            ret += "; victim_place=" + std::to_string(victim_place);
            ret += "; thief_place=" + std::to_string(thief_place);
            ret += "; team_count=" + std::to_string(team_count);
            ret += "; expected=" + std::to_string(expected);
            return ret;
        }
        int base_points;
        int victim_place;
        int thief_place;
        int team_count;
        int expected;
    };

    std::vector<LocalDataTestRuCtf> tests;
    tests.push_back(LocalDataTestRuCtf(100, 1, 2, 2, 100));
    tests.push_back(LocalDataTestRuCtf(100, 2, 2, 3, 100));
    tests.push_back(LocalDataTestRuCtf(100, 3, 2, 4, 66));
    tests.push_back(LocalDataTestRuCtf(100, 4, 2, 5, 50));
    tests.push_back(LocalDataTestRuCtf(100, 5, 2, 5, 25));
    tests.push_back(LocalDataTestRuCtf(100, 10, 2, 5, 0));
    tests.push_back(LocalDataTestRuCtf(100, 6, 2, 25, 83));
    tests.push_back(LocalDataTestRuCtf(100, 24, 1, 25, 4));

    int ret = 0;
    for (int i = 0; i < tests.size(); ++i) {
        LocalDataTestRuCtf t = tests[i];
        int got = formula->calc_stolen(t.base_points, t.victim_place, t.thief_place, t.team_count);
        int i_got = got;
        int i_expected = t.expected;
        if (i_got != i_expected) {
            std::string msg = t.to_string();
            msg += "; got=" + std::to_string(got);
            std::cerr << "ERROR: " << msg << std::endl;
            ret++;
        }
    }
    if (ret == 0) {
        std::cout << "ok" << std::endl;
    }
    return ret;
}