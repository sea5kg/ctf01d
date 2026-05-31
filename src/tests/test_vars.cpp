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
#include <memory>
#include <wsjcpp_core.h>
#include "ctf01d/objects/ctf01d_var.h"

int main() {
  std::string sTestYaml =
    "# Some comment 1\n"
    "game: \n"
    "  cost: 3\n"
    "  id: my_game\n"
    "\n" // empty line
  ;

  std::vector<std::shared_ptr<ctf01d::var>> scope_vars;

  WsjcppYaml yaml;
  std::string err;

  if (!yaml.loadFromString("test_config.yaml", sTestYaml, err)) {
    std::cerr << "Error parsing " << err << std::endl;
    return -1;
  }

  // int
  {
    ctf01d::var_int varCost({"game", "cost"}, 10);
    if (varCost.type() != ctf01d::var_type::INTEGER) {
      std::cerr << "FAILED: Expected integer type" << std::endl;
      return -1;
    }
    if (varCost.name() != "game.cost") {
      std::cerr << "FAILED: Expected name 'game.cost', but got '" << varCost.name() << "'" << std::endl;
      return -1;
    }
    if (varCost.value() != 10) {
      std::cerr << "FAILED: Expected value 10 but got " << std::to_string(varCost.value()) << "'" << std::endl;
      return -1;
    }
    if (!varCost.set_value(12, err)) {
      std::cerr << "FAILED: Problem with set value" << std::endl;
      return -1;
    }
    if (varCost.value() != 12) {
      std::cerr << "FAILED: Expected value 12 but got " << std::to_string(varCost.value()) << "'" << std::endl;
      return -1;
    }
    auto cursor = yaml.getCursor();
    if (!varCost.read(cursor, err)) {
      std::cerr << "FAILED: Could not read from yaml" << std::endl;
      return -1;
    }
    if (varCost.value() != 3) {
      std::cerr << "FAILED: Expected value 3 but got " << std::to_string(varCost.value()) << "'" << std::endl;
      return -1;
    }
  }

  // string
  {
    auto varGameId = ctf01d::var_string::create({"game", "id"}, "test", scope_vars);
    if (varGameId->type() != ctf01d::var_type::STRING) {
      std::cerr << "FAILED: Expected string type" << std::endl;
      return -1;
    }
    if (varGameId->name() != "game.id") {
      std::cerr << "FAILED: Expected name 'game.id', but got '" << varGameId->name() << "'" << std::endl;
      return -1;
    }
    if (varGameId->value() != "test") {
      std::cerr << "FAILED: Expected value 'test' but got " << varGameId->value() << "'" << std::endl;
      return -1;
    }
    varGameId->set_value("hello");
    if (varGameId->value() != "hello") {
      std::cerr << "FAILED: Expected value 'hello' but got " << varGameId->value() << "'" << std::endl;
      return -1;
    }
    auto cursor = yaml.getCursor();
    if (!varGameId->read(cursor, err)) {
      std::cerr << "FAILED: Could not read from yaml" << std::endl;
      return -1;
    }
    if (varGameId->value() != "my_game") {
      std::cerr << "FAILED: Expected value 'my_game' but got " << varGameId->value() << "'" << std::endl;
      return -1;
    }

  }

  std::cout << "ok" << std::endl;
  return 0;
}