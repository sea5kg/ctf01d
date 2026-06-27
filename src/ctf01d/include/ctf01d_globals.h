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

namespace ctf01d {

static const int MAX_FLAG_LIFETIME_SECONDS = 1500;
static const int MAX_FLAG_COST_IN_POINTS = 1000;
static const int MIN_TCP_PORT = 11;
static const int MAX_TCP_PORT = 65435;
static const int DEFAULT_FLAG_LIFETIME_IN_SECONDS = 60;

class json_fields {
public:
  inline static const std::string SUMMARY_ACTIVITIES = "sum_act";
  inline static const std::string TRIES = "tries";
  inline static const std::string ID = "id";
  inline static const std::string NAME = "name";
  inline static const std::string DESCRIPTION = "dsc";
  inline static const std::string UPDATED = "upd";
  inline static const std::string IP_OR_HOST = "ip_address";
  inline static const std::string LOGO = "logo";
  inline static const std::string LOGO_BIG = "logo-big";
  inline static const std::string CTF01D_VERSION = "ctf01d-version";
  inline static const std::string CONFIG_UPDATED = "c";
  inline static const std::string CURRENT_TIME = "t";
};

class yaml_keys {
public:
  inline static const std::string GAME = "game";
  inline static const std::string ID = "id";
  inline static const std::string NAME = "name";
  inline static const std::string START_UTC = "start-utc";
  inline static const std::string END_UTC = "end-utc";
  inline static const std::string COFFEE_BREAK = "coffee-break";
  inline static const std::string FLAG_LIFETIME_IN_SECONDS = "flag-lifetime-in-seconds";
  inline static const std::string FLAG_COST_IN_POINTS = "flag-cost-in-points";
  inline static const std::string TYPE = "type";
  inline static const std::string TYPE_NORMAL = "normal";
  inline static const std::string TYPE_RED = "red";
  inline static const std::string TYPE_BLUE = "blue";
  inline static const std::string TYPE_QUEST = "guest";
  inline static const std::string TYPE_DISQUALIFIED = "disqualified";
  inline static const std::string DESCRIPTION = "description";
  inline static const std::string ACTIVE = "active";
  inline static const std::string LOGO = "logo";
  inline static const std::string LOGO_BIG = "logo-big";
  inline static const std::string IP_OR_HOST = "ip-or-host";
};

} // namespace ctf01d
