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
#include <thread>
#include <chrono>
#include "ctf01d_files_watcher.h"

int main() {

  std::shared_ptr<ctf01d::files_watcher> watcher = std::make_shared<ctf01d::files_watcher>();

  if (!WsjcppCore::dirExists("test_file_watcher")) {
    if (!WsjcppCore::makeDir("test_file_watcher")) {
      std::cerr << "Could not create 'test_file_watcher' directory" << std::endl;
      return -1;
    }
  }

  WsjcppCore::initRandom();
  const std::string &alphabet = wsjcpp::Core::englishAlphabetBothCaseAndNumbers();

  std::string file_1_txt = "test_file_watcher/1.txt";
  WsjcppCore::writeFile(file_1_txt, "test1 " + wsjcpp::Core::randomString(alphabet, 10));
  watcher->watchFile(file_1_txt);
  // std::cout << "LastModifiedTimeFile: " << watcher->getLastModifiedTimeFile(file_1_txt) << std::endl;

  std::string file_2_txt = "test_file_watcher/2.txt";
  WsjcppCore::writeFile(file_2_txt, "test2 " + wsjcpp::Core::randomString(alphabet, 10));
  watcher->watchFile(file_2_txt);

  std::string file_3_txt = "test_file_watcher/3.txt";
  WsjcppCore::writeFile(file_3_txt, "test3 " + wsjcpp::Core::randomString(alphabet, 10));
  watcher->watchFile(file_3_txt);

  if (watcher->isModifiedFile(file_1_txt)) {
    std::cerr << "Wrong state. File must be not modified" << std::endl;
    return -1;
  }
  // std::cout << "LastModifiedTimeFile: " << watcher->getLastModifiedTimeFile(file_1_txt) << std::endl;
  // wait one seconds because linux modified time
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  WsjcppCore::writeFile(file_1_txt, "test1 " + wsjcpp::Core::randomString(alphabet, 20));
  WsjcppCore::writeFile(file_2_txt, "test2 " + wsjcpp::Core::randomString(alphabet, 20));
  // std::cout << "LastModifiedTimeFile: " << watcher->getLastModifiedTimeFile(file_1_txt) << std::endl;

  if (!watcher->isModifiedFile(file_1_txt)) {
    std::cerr << "Expected that file was modified." << std::endl;
    return -1;
  }

  std::map<std::string, long> modified_files = watcher->get_modified_files();
  if (modified_files.size() != 1) {
    std::cerr << "Expected only one file modified." << std::endl;
    return -1;
  }
  for (auto it = modified_files.begin(); it != modified_files.end(); ++it) {
    const std::string &filepath = it->first;
    if (filepath != file_2_txt) {
      std::cerr << "Expected modified file '" << file_2_txt << "', but got '" << filepath << "'." << std::endl;
      return -1;
    }
  }

  return 0;
}
