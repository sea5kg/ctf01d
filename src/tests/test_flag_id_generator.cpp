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
#include "ctf01d/utils/ctf01d_flag_id_generators.h"

int main() {
  const int random_string_size = 12;
  nlohmann::json options;
  options["size"] = random_string_size;
  auto gen_random_string = ctf01d::flag_id_generators::random_string(options);

  static const std::string alphabet =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

  std::map<char, int> _stat;
  for (int i = 0; i < alphabet.size(); ++i) {
    _stat[alphabet[i]] = 0;
  }
  int tests = 100;
  int test = 0;
  while (test < tests) {
    test++;
    std::string s = gen_random_string->generate();
    if (s.size() != random_string_size) {
      std::cerr << "Expected size of string " << random_string_size << ", but got " << s.size() << std::endl;
      return 1;
    }
    for (int i = 0; i < s.size(); i++) {
      _stat[s[i]]++;
    }
  }
  int _stat_0 = 0;
  std::string _stat_0_values = "";
  for (const auto& [key, value] : _stat) {
    if (value == 0) {
      _stat_0++;
      _stat_0_values += key;
    }
  }

  if (_stat_0 > 0) {
    std::cerr << "Expected non-zero for values " << _stat_0_values << std::endl;
    return 1;
  }

  std::cout << "ok" << std::endl;
  return 0;
}
