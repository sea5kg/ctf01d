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
import time
import random
import psutil
import requests
from .utils_log import UtilsLog
from .utils_tests import UtilsTests
from .pm_config import PmConfig


class CommandTests:
    """ CommandTests """
    def __init__(self, _: PmConfig):
        self.__log = UtilsLog("CommandTests").get_logger()
        # self.__config = config
        self.__subcommand_name = "tests"
        self.__tests = {
            "path_traversal": self.__test_path_traversal,
            "memory_leak": self.__test_memory_leak,
            "send_random_flags": self.__test_send_random_flags,
        }

    def get_name(self):
        """ return subcommand name """
        return self.__subcommand_name

    def do_registry(self, subparsers):
        """ registering sub command """
        _parser_tests = subparsers.add_parser(
            name=self.__subcommand_name,
            description='Run tests'
        )
        _parser_tests.add_argument(
            '-ls', '--list',
            dest='show_list_tests',
            help='Show list tests',
            action='store_true',
        )
        _parser_tests.add_argument(
            '-r', '--run',
            dest='test_name',
            help='Specify name of test'
        )
        _parser_tests.set_defaults(subparser=self.__subcommand_name)

    def __test_path_traversal(self):
        self.__log.info("Run test 'Path Traversal'.")
        _, pid = UtilsTests.start_empty_jury_1x3(self.__log)
        self.__log.info("Try request to jury")
        try:
            url_db_flags_live = 'http://localhost:8080/../db/flags_live.db'
            _session = requests.Session()
            _req = requests.Request(method='GET', url=url_db_flags_live)
            _req_prep = _req.prepare()
            _req_prep.url = url_db_flags_live
            _resp = _session.send(_req_prep)
            if _resp.status_code == 200:
                self.__log.error(
                    "\n"
                    "\n************************************************"
                    "\n* Vulnerability 'Path Traversal' FOUND (!!!!!) *"
                    "\n************************************************"
                    "\n"
                )
                sys.exit(1)
            self.__log.info(">>>> Everything fine!")
        finally:
            UtilsTests.stop_jury(pid)

    def __test_memory_leak(self):
        self.__log.info("Run test 'Path Traversal'.")
        _, pid = UtilsTests.start_empty_jury_1x3(self.__log)
        self.__log.info("Try request to jury")
        _proc = psutil.Process(pid)
        _first_mem_rss_mb = 0
        for i in range(1, 2):
            _first_mem_rss_mb = _proc.memory_info().rss / 1024
            print(_first_mem_rss_mb)
            time.sleep(0.5)

        _urls = {
            "http://localhost:8080/api/v1/myip": 200,
            "http://localhost:8080/flag?teamid=t02&flag=c01d4567-e89b-12d3-a456-426600000010": 403,
            "http://localhost:8080/api/v1/teams": 200,
            "http://localhost:8080/api/v1/services": 200,
            "http://localhost:8080/api/v1/scoreboard": 200,
            "http://localhost:8080/team-logo/t01": 200,
            "http://localhost:8080/team-logo/t02": 200,
            "http://localhost:8080/team-logo/t03": 200,
            "http://localhost:8080/api/v1/game": 200,
        }
        try:
            for _url, _expected_status_code in _urls.items():
                print(_url)
                _before_mem_rss_kb = _proc.memory_info().rss / 1024
                print("Before:", _before_mem_rss_kb)
                for i in range(1, 50):
                    print(i)
                    _resp = requests.get(_url, timeout=0.2)
                    if _resp.status_code != _expected_status_code:
                        err_msg = _url + " response: " + str(_resp.text)
                        err_msg += "Expected: " + str(_expected_status_code)
                        err_msg += ", but got " + str(_resp.status_code)
                        UtilsTests.print_error_and_exit(self.__log, err_msg)
                _after_mem_rss_kb = _proc.memory_info().rss / 1024
                print("After:", _after_mem_rss_kb)
                print("Diff:", _after_mem_rss_kb - _before_mem_rss_kb)
            self.__log.info(">>>> Everything fine!")
        except requests.exceptions.Timeout:
            UtilsTests.print_error_and_exit(self.__log, "Timed out")
        finally:
            UtilsTests.stop_jury(pid)

    def __test_send_random_flags(self):
        self.__log.info("Run test 'Path Traversal'.")
        _, pid = UtilsTests.start_empty_jury_1x3(self.__log)
        self.__log.info("Try request to jury")
        _teams = []
        try:
            _resp = requests.get("http://localhost:8080/api/v1/teams", timeout=0.2)
            if _resp.status_code != 200:
                UtilsTests.print_error_and_exit(self.__log, "Could not get info about teams")
            _teams = _resp.json()["teams"]
        except requests.exceptions.Timeout:
            UtilsTests.print_error_and_exit(self.__log, "Timed out api/v1/teams")
        i = 0
        try:
            for _team in _teams:
                _team["activity"] = 0
            # for _team in _teams:
            #     print(_team)
            sended_flags = 0
            while i < 140:
                i = i + 1
                sended_flags += 1
                team_i = random.randint(0, len(_teams)-1)
                team_id = _teams[team_i]["id"]
                _teams[team_i]["activity"] += 1
                _fl_url = 'http://localhost:8080/flag?teamid=' + str(team_id)
                _fl_url += '&flag=' + UtilsTests.random_flag()

                # print("Request " + http_get_url)
                _resp = requests.get(_fl_url, timeout=0.2)
                if _resp.status_code not in [403, 200]:
                    UtilsTests.print_error_and_exit(
                        self.__log,
                        "Shit happen " + str(_resp.status_code)
                    )
            _resp = requests.get("http://localhost:8080/api/v1/scoreboard", timeout=0.2)
            if _resp.status_code != 200:
                UtilsTests.print_error_and_exit(self.__log, "Could not get info about teams")
            _score = _resp.json()
            if _score["sum_act"] != sended_flags:
                UtilsTests.print_error_and_exit(self.__log, "Wrong sum_act")
            self.__log.info("sum_act %s OK", str(_score["sum_act"]))
            for _team in _teams:
                team_id = _team["id"]
                if _score['scoreboard'][team_id]['tries'] != _team["activity"]:
                    UtilsTests.print_error_and_exit(
                        self.__log,
                        "Wrong tries for " + team_id
                    )
                self.__log.info("%s tries %s OK", team_id, str(_team["activity"]))
            self.__log.info(">>>> Everything fine!")
        except requests.exceptions.Timeout:
            UtilsTests.print_error_and_exit(self.__log, "Timed out on " + str(i))
        finally:
            UtilsTests.stop_jury(pid)

    def execute(self, args):
        """ executing """
        if args.show_list_tests:
            self.__log.info("Tests: \n - %s\n", "\n - ".join(self.__tests.keys()))
            sys.exit(0)

        if args.test_name not in self.__tests:
            self.__log.info(
                "Not found test '%s' to run. \n  Note: %s",
                args.test_name,
                "Please use -ls argument for looking list of tests."
            )
            sys.exit(1)
        self.__log.info("Running test %s ...", args.test_name)
        self.__tests[args.test_name]()

        sys.exit(0)
