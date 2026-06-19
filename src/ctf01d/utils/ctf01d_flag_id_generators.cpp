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


#include "ctf01d_flag_id_generators.h"
#include <chrono>
#include <openssl/rand.h>

#include "sea5kg_logger.h"

namespace ctf01d {

class flag_id_generator_random_string : public flag_id_generator {
public:
  flag_id_generator_random_string(nlohmann::json options);
  virtual std::string generate() override;
private:
  int m_size;
};

flag_id_generator_random_string::flag_id_generator_random_string(nlohmann::json options) {
  m_size = 10;
  if (options.contains("size")) {
    m_size = options["size"];
  }
}

std::string flag_id_generator_random_string::generate() {
  std::string ret;
  // static const std::string sAlphabet =
  //   "0123456789"
  //   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  //   "abcdefghijklmnopqrstuvwxyz";
  while (ret.size() < m_size) {
    // ret += sAlphabet[rand() % sAlphabet.length()];
    const int buffer_size = 16;
    unsigned char buffer[buffer_size];
    if (RAND_bytes(buffer, buffer_size) != 1) {
      sea5kg::log::throw_err("flag_id_generator_random_string", "Problem with RAND_bytes");
    }
    for (int i = 0; i < buffer_size; ++i) {
      if (ret.size() >= m_size) {
        break;
      }
      char c = buffer[i];
      if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
        ret += c;
      }
    }
  }
  return ret;
}

std::shared_ptr<flag_id_generator> flag_id_generators::random_string(nlohmann::json options) {
  return std::make_shared<flag_id_generator_random_string>(options);
}

} // namespace ctf01d
