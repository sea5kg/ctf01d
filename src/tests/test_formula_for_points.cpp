#include <vector>
#include <iostream>
#include <wsjcpp_core.h>
#include <memory>
#include "ctf01d_formulas_for_points.h"

int main() {

    std::shared_ptr<Ctf01dFormulasForPoints> formula = std::make_shared<Ctf01dFormulasForPoints_RuCtf>();

    struct LocalDataTestRuCtf {
        LocalDataTestRuCtf(float base_points, int victim_place, int thief_place, int team_count, float expected)
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
        float base_points;
        int victim_place;
        int thief_place;
        int team_count;
        float expected;
    };

    std::vector<LocalDataTestRuCtf> tests;
    tests.push_back(LocalDataTestRuCtf(10.0, 1, 2, 2, 10.0));
    tests.push_back(LocalDataTestRuCtf(10.0, 2, 2, 3, 10.0));
    tests.push_back(LocalDataTestRuCtf(10.0, 3, 2, 4, 6.6));
    tests.push_back(LocalDataTestRuCtf(10.0, 4, 2, 5, 5.0));
    tests.push_back(LocalDataTestRuCtf(10.0, 5, 2, 5, 2.5));
    tests.push_back(LocalDataTestRuCtf(10.0, 10, 2, 5, 0.0));
    tests.push_back(LocalDataTestRuCtf(10.0, 6, 2, 25, 8.3));
    tests.push_back(LocalDataTestRuCtf(10.0, 24, 1, 25, 0.4));

    int ret = 0;
    for (int i = 0; i < tests.size(); ++i) {
        LocalDataTestRuCtf t = tests[i];
        float got = formula->calcStolen(t.base_points, t.victim_place, t.thief_place, t.team_count);
        int i_got = int(got*10);
        int i_expected = int(t.expected*10);
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