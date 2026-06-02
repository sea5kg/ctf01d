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

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <wsjcpp_yaml.h>
#include "ctf01d_var.h"

namespace ctf01d {

class service_config {
public:
  service_config();

  bool read(WsjcppYamlCursor &cursor, const std::string &work_dir, std::string &err);
  std::string id() const;
  std::string name() const;
  std::string scriptPath() const;
  std::string scriptDir() const;
  bool isEnabled() const;
  int scriptWaitInSec() const;
  int round_in_seconds() const;

private:
  std::string TAG;
  ctf01d::scope_vars m_vars = ctf01d::scope_vars("service_config");
  std::string m_work_dir;
  std::shared_ptr<ctf01d::var_string> m_id;
  std::shared_ptr<ctf01d::var_string> m_name;
  std::shared_ptr<ctf01d::var_bool> m_enabled;
  std::shared_ptr<ctf01d::var_file> m_logo;
  std::shared_ptr<ctf01d::var_file> m_big_logo;
  std::shared_ptr<ctf01d::var_int> m_script_wait_in_sec;
  std::shared_ptr<ctf01d::var_int> m_round_in_seconds;
  std::shared_ptr<ctf01d::var_string> m_script_path;
  std::shared_ptr<ctf01d::var_dir> m_script_dir;
};

} // namespace ctf01d