#!/usr/bin/env python3
##################################################################################
#           Project
#   _______ _________ _______  _______  __    ______
#  (  ____ \\__   __/(  ____ \(  __   )/  \  (  __  \
#  | (    \/   ) (   | (    \/| (  )  |\/) ) | (  \  )
#  | |         | |   | (__    | | /   |  | | | |   ) |
#  | |         | |   |  __)   | (/ /) |  | | | |   | |
#  | |         | |   | (      |   / | |  | | | |   ) |
#  | (____/\   | |   | )      |  (__) |__) (_| (__/  )
#  (_______/   )_(   |/       (_______)\____/(______/
#
# MIT License
#
# Copyright (c) 2018-2026 Evgenii Sopov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Original repository: https://github.com/sea5kg/ctf01d
#
##################################################################################

""" Utils Ctf01d Config """

import os
import yaml
from .utils_files import UtilsFiles


class UtilsCtf01dConfig:
    """ UtilsCtf01dConfig """

    @staticmethod
    def generate_default(count_teams, count_services):
        """ generate default config """
        ret = {
            "game": {
                "id": "test",
                "name": "Test First Game",
                "start_utc": "2023-11-12 16:00:00",
                "end_utc": "2030-11-12 22:00:00",
                "coffee_break_start_utc": "2023-11-12 20:00:00",
                "coffee_break_end_utc": "2023-11-12 21:00:00",
                "flag_lifetime_in_seconds": 1,
                "basic_costs_stolen_flag_in_points": 1,
                "cost_defense_flag_in_points": 1.0,
            },

            "scoreboard": {
                "port": 8080,
                "html-dir-path": "./html",
                "random": False,
            },

            "checkers": [],
            "teams": [],
        }
        i = 0
        while i < count_teams:
            i += 1
            n = str(i)
            n00 = n.rjust(2, '0')
            ret["teams"].append({
                "id": "t" + n00,
                "name": "Team #" + n,
                "active": True,
                "logo": "./html/images/teams/team" + n00 + ".png",
                "ip_address": "127.0." + n + ".1",
            })
        i = 0
        while i < count_services:
            i += 1
            n = str(i)
            ret["checkers"].append({
                "id": "test_service" + n,
                "name": "Service" + n,
                "enabled": True,
                # "host": 127.0.0.1,
                # "port": 10001,
                "script-relative-path": "./checker.py",
                "script-timeout-in-seconds": 5,
                "round-in-seconds": 15,
            })
        return ret

    @staticmethod
    def write_to_file(_cfg_dir, cfg):
        """ write_to_file """
        _output = os.path.join(_cfg_dir, "config.yml")
        with open(_output, 'wt', encoding="utf-8") as file:
            # yaml.dump(cfg, file)
            yaml.dump(
                cfg,
                file,
                default_flow_style=False,
                sort_keys=False
            )

    @staticmethod
    def write_checker_test(_cfg_dir, service_name):
        """ extract from data example checker.py """
        _checker_dir = os.path.join(_cfg_dir, "checker_" + service_name)
        if not os.path.isdir(_checker_dir):
            os.makedirs(_checker_dir, exist_ok=True)
        script_dir = os.path.dirname(os.path.abspath(__file__))
        script_dir = os.path.join(script_dir, "data")
        checker_res_path = os.path.join(script_dir, "checker_example_service.py")
        _lines = UtilsFiles.safe_read_file(checker_res_path)
        _checker_output = os.path.join(_checker_dir, "checker.py")
        with open(_checker_output, "wt", encoding="utf-8") as _file:
            _file.write("".join(_lines))
        os.system("chmod +x " + _checker_output)

    @staticmethod
    def write_example_config_1x3(_cfg_dir):
        """ extract from data example config.yml """
        script_dir = os.path.dirname(os.path.abspath(__file__))
        script_dir = os.path.join(script_dir, "data")
        config_res_path = os.path.join(script_dir, "config_1x3.yml")
        _lines = UtilsFiles.safe_read_file(config_res_path)
        _config_output = os.path.join(_cfg_dir, "config.yml")
        with open(_config_output, "wt", encoding="utf-8") as _file:
            _file.write("".join(_lines))
