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

""" subcommand tests for run tests """

import sys
import os
from .utils_log import UtilsLog
from .utils_files import UtilsFiles
from .pm_config import PmConfig


class CommandMakeTestGame:
    """ CommandMakeTestGame """
    def __init__(self, config: PmConfig):
        self.__log = UtilsLog("CommandMakeTestGame").get_logger()
        self.__config = config
        self.__subcommand_name = "make-test-game"

    def get_name(self):
        """ return subcommand name """
        return self.__subcommand_name

    def do_registry(self, subparsers):
        """ registering sub command """
        _parser_make_test_game = subparsers.add_parser(
            name=self.__subcommand_name,
            description='Make a dir with configs for test game'
        )
        _parser_make_test_game.add_argument(
            '-t', '--teams',
            type=int,
            dest='number_of_teams',
            help='Number of Teams',
            required=True,
        )
        _parser_make_test_game.add_argument(
            '-s', '--services',
            type=int,
            dest='number_of_services',
            help='Number of Services',
            required=True,
        )
        _parser_make_test_game.set_defaults(subparser=self.__subcommand_name)

    def __prepare_test_game_dir(self):
        _dir = os.path.join(self.__config.get_root_dir(), "test_game_config")
        if os.path.isdir(_dir):
            UtilsFiles.recursive_remove_files(_dir)
        if os.path.isdir(_dir):
            self.__log.error("Did not removed %s", _dir)
            sys.exit(1)
        os.makedirs(_dir, exist_ok=True)
        return _dir

    def __prepare_make_vulnbox(self, _dir, number_of_services):
        _vulnbox_dir = os.path.join(_dir, "vulnbox")
        os.makedirs(_vulnbox_dir, exist_ok=True)
        _compose = [
            "version: '3'",
            "services:",
        ]
        _readme = [
            "# Test Game. VulnBox",
            "",
            "Start: `docker compose up -d`",
            "",
            "## Services",
            "",
        ]
        for service_i in range(1, number_of_services+1):
            _port = 4100 + service_i
            _readme.append("- Service" + str(service_i) + " port: " + str(_port))
            _compose.extend([
                "  vulnbox_service" + str(service_i) + ":",
                "    image: python:latest",
                "    user: \"${UID}:${GID}\"",
                "    volumes:",
                "      - \"./tmp/flags_service" + str(service_i) + ":/root/flags\"",
                "      - \"./vuln_service.py:/root/vuln_service.py\"",
                "    command: sh -c \"python3 -u /root/vuln_service.py " + str(_port) + "\"",
                "    expose:",
                "      - \"" + str(_port) + "\"",
                "    ports:",
                "      - \"" + str(_port) + ":" + str(_port) + "\"",
                "    restart: always",
                "    networks:",
                "      - vulnbox_net",
                "",
            ])
        _readme.append("")
        _compose.extend([
            "networks:",
            "  vulnbox_net:",
            "    driver: bridge",
            "",
        ])

        _vuln_service_py_path = os.path.join(_vulnbox_dir, "vuln_service.py")
        UtilsFiles.extract_file_from_res("vuln_example_service.py", _vuln_service_py_path)

        _readme_path = os.path.join(_vulnbox_dir, "README.md")
        with open(_readme_path, "wt", encoding="utf-8", newline="\n") as _file:
            _file.write("\n".join(_readme))

        _compose_path = os.path.join(_vulnbox_dir, "docker-compose.yml")
        with open(_compose_path, "wt", encoding="utf-8", newline="\n") as _file:
            _file.write("\n".join(_compose))

    def __prepare_checker(self, _data_path, _name_service, _port):
        _checker_dir = os.path.join(_data_path, "checker_" + _name_service)
        os.makedirs(_checker_dir, exist_ok=True)
        _checker_path = os.path.join(_checker_dir, "checker.py")
        UtilsFiles.extract_file_from_res("checker_example_service.py", _checker_path)
        # replace port in checker
        _lines = UtilsFiles.safe_read_file(_checker_path)
        _new_lines = []
        for _line in _lines:
            if _line.strip().startswith("port = "):
                _new_lines.append("port = " + str(_port) + "\n")
            else:
                _new_lines.append(_line)
        with open(_checker_path, "wt", encoding="utf-8", newline="\n") as _checker:
            _checker.write("".join(_new_lines))

    def __prepare_make_jury(self, _dir, number_of_teams, number_of_services):
        _jury_dir = os.path.join(_dir, "jury")
        os.makedirs(_jury_dir, exist_ok=True)
        # in future somehow get latest version
        _compose = [
            "version: '3'",
            "services:",
            "  ctf01d_jury:",
            "    container_name: ctf01d_jury_my_game",
            "    image: sea5kg/ctf01d:v0.7.0",
            "    volumes:",
            "      - \"./data:/usr/share/ctf01d\"",
            "    environment:",
            "      CTF01D_WORKDIR: \"/usr/share/ctf01d\"",
            "    ports:",
            "      - \"8080:8080\"",
            "    restart: always",
            "    networks:",
            "      - ctf01d_net",
            "",
            "networks:",
            "  ctf01d_net:",
            "    driver: bridge",
            "",
        ]
        _readme = [
            "# Test Game. Jury",
            "",
            "Start: `docker compose up -d`",
            "",
            "## Services",
            "",
        ]
        _config = [
            "game:",
            "  id: \"test_game\"",
            "  name: \"Test Game\"",
            "  start_utc: \"2023-11-12 16:00:00\"",
            "  end_utc: \"2030-11-12 22:00:00\"",
            "  coffee_break_start: \"2023-11-12 20:00:00\"",
            "  coffee_break_end: \"2023-11-12 21:00:00\"",
            "  flag_lifetime_in_seconds: 1",
            "  flag_cost_in_points: 100",
            "",
            "scoreboard:",
            "  port: 8080",
            "  htmlfolder: \"./html\"",
            "  random: no",
            "",
            "checkers:",
        ]

        _data_path = os.path.join(_jury_dir, "data")
        os.makedirs(_data_path, exist_ok=True)
        for service_i in range(1, number_of_services+1):
            _name_service = "service" + str(service_i)
            _config.extend([
                "  - id: \"" + _name_service + "\"",
                "    service_name: \"Service" + str(service_i) + "\"",
                "    enabled: yes",
                "    script_path: \"./checker.py\"",
                "    script_wait_in_sec: 5",
                "    round_in_seconds: 15",
            ])
            _port = 4100 + service_i
            _readme.append("- Service" + str(service_i) + " port: " + str(_port))
            self.__prepare_checker(_data_path, _name_service, _port)
        _readme.extend([
            "",
            "## Teams",
            "",
        ])
        _config.extend([
            "",
            "teams:",
        ])

        for team_i in range(1, number_of_teams+1):
            team_ti = str(team_i).rjust(2, '0')
            _readme.append(
                "- Team # " + str(team_i) + " t" + team_ti + ": \"10.10." + str(team_i) + ".3\""
            )
            _config.extend([
                "  - id: \"t" + team_ti + "\"",
                "    name: \"Team #" + str(team_i) + "\"",
                "    active: yes",
                "    logo: \"./html/images/teams/team" + team_ti + ".png\"",
                "    ip_address: \"10.10." + str(team_i) + ".3\"",
            ])
        _config.extend([""])
        _readme.extend([""])
        UtilsFiles.write_file(os.path.join(_jury_dir, "README.md"), _readme)
        UtilsFiles.write_file(os.path.join(_jury_dir, "docker-compose.yml"), _compose)
        UtilsFiles.write_file(os.path.join(_data_path, "config.yml"), _config)

    def __prepare_make_attacker(self, _dir):
        _attacker_dir = os.path.join(_dir, "attacker")
        os.makedirs(_attacker_dir, exist_ok=True)
        _compose = [
            "version: '3'",
            "services:",
            "  attacker:",
            "    build:",
            "      context: .",
            "      dockerfile: Dockerfile",
            "    image: test_game_config_attacker:latest",
            "    volumes:",
            "    - \"./tmp/flags_exploit:/root/flags\"",
            "    - \"./attacker.py:/root/attacker.py\"",
            "    command: sh -c \"python3 -u /root/attacker.py 10.10.100.100:8080\"",
            "    restart: always",
            "    networks:",
            "    - attacker_net",
            "",
            "networks:",
            "  attacker_net:",
            "    driver: bridge",
            "",
        ]
        _readme = [
            "# Test Game. Attacker",
            "",
            "Start: `docker compose up -d`",
            "",
        ]

        UtilsFiles.extract_file_from_res(
            "Dockerfile.attacker",
            os.path.join(_attacker_dir, "Dockerfile")
        )
        _attacker_path = os.path.join(_attacker_dir, "attacker.py")
        UtilsFiles.extract_file_from_res("attacker_example_service.py", _attacker_path)
        UtilsFiles.write_file(os.path.join(_attacker_dir, "README.md"), _readme)
        UtilsFiles.write_file(os.path.join(_attacker_dir, "docker-compose.yml"), _compose)

    def execute(self, args):
        """ executing """
        self.__log.info(
            "\n*****************************\n"
            "Make a test game configuration:\n"
            "  teams: %s\n"
            "  services %s\n"
            "*****************************\n",
            str(args.number_of_teams),
            str(args.number_of_services),
        )
        _dir = self.__prepare_test_game_dir()

        self.__prepare_make_vulnbox(_dir, args.number_of_services)
        self.__prepare_make_jury(_dir, args.number_of_teams, args.number_of_services)
        self.__prepare_make_attacker(_dir)
        self.__log.info("DONE")
        sys.exit(0)
