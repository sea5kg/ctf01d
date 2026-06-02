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
#include <wsjcpp_yaml.h>
#include "ctf01d_var.h"

namespace ctf01d {

class team_config {
public:
  team_config();
  bool read(WsjcppYamlCursor &cursor, const std::string &work_dir, std::string &err);

  void setId(const std::string &sId);
  const std::string &getId() const;

  void setName(const std::string &sName);
  const std::string &getName() const;

  void setIpAddress(const std::string &sIpAddress);
  const std::string &ipAddress() const;

  void setActive(bool bActive);
  bool isActive() const;

  void setLogo(const std::string &sLogo);
  const std::string &logo() const;

  int getLogoLastWriteTime();

private:
  std::string TAG;
  std::string m_work_dir;
  ctf01d::scope_vars m_vars = ctf01d::scope_vars("team_config");
  std::shared_ptr<ctf01d::var_string> m_type;
  std::shared_ptr<ctf01d::var_file> m_logo;
  std::shared_ptr<ctf01d::var_file> m_big_logo;

  bool m_bActive;
  std::string m_sTeamID;
  std::string m_sName;
  std::string m_sIpAddress;
  std::string m_sLogo;
  int m_nLogoLastWriteTime;
};

} // namespace ctf01d