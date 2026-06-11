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
from .utils_tests import UtilsTests, StartJuryTest
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
            '-nrj', '--no-run-jury',
            dest='run_jury',
            help='No run jury',
            default=True,
            action='store_false',
        )
        _parser_tests.add_argument(
            '-r', '--run',
            dest='test_name',
            help='Specify name of test'
        )
        _parser_tests.set_defaults(subparser=self.__subcommand_name)

    def __test_path_traversal(self, run_jury):
        self.__log.info("Run test 'Path Traversal'.")
        with StartJuryTest(run_jury, self.__log) as _test:
            try:
                url_db_flags_live = 'http://localhost:8080/../db/flags_live.db'
                _session = requests.Session()
                _req = requests.Request(method='GET', url=url_db_flags_live)
                _req_prep = _req.prepare()
                _req_prep.url = url_db_flags_live
                _resp = _session.send(_req_prep)
                if _resp.status_code == 200:
                    _test.err_exit("Vulnerability 'Path Traversal' FOUND (!!!!!)")
                self.__log.info(">>>> Everything fine!")
            except requests.exceptions.Timeout:
                _test.err_exit("Timed out")
            # finally:
            #     UtilsTests.stop_jury(run_jury, pid)

    def __test_memory_leak(self, run_jury):
        self.__log.info("Run test 'Memory Leak'.")
        with StartJuryTest(run_jury, self.__log) as _test:
            _proc = psutil.Process(_test.get_pid())
            _first_mem_rss_mb = 0
            for i in range(1, 2):
                _first_mem_rss_mb = _proc.memory_info().rss / 1024
                print(_first_mem_rss_mb)
                time.sleep(0.5)

            _urls = {
                "http://localhost:8080/api/v1/my-ip": 200,
                "http://localhost:8080/flag?team_id=t02&flag=" + UtilsTests.random_flag(): 403,
                "http://localhost:8080/api/v1/teams": 200,
                "http://localhost:8080/api/v1/services": 200,
                "http://localhost:8080/api/v1/scoreboard": 200,
                "http://localhost:8080/team-logo/t01": 200,
                "http://localhost:8080/team-logo/t02": 200,
                "http://localhost:8080/team-logo/t03": 200,
                "http://localhost:8080/api/v1/game": 200,
                "http://localhost:8080/api/v1/game/current-time": 200,
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
                            _test.err_exit(err_msg)
                    _after_mem_rss_kb = _proc.memory_info().rss / 1024
                    print("After:", _after_mem_rss_kb)
                    print("Diff:", _after_mem_rss_kb - _before_mem_rss_kb)
                self.__log.info(">>>> Everything fine!")
            except requests.exceptions.Timeout:
                _test.err_exit("Timed out")
            # finally:
            #     UtilsTests.stop_jury(run_jury, pid)

    def __test_send_random_flags(self, run_jury):
        self.__log.info("Run test 'Send Random Flags'.")
        with StartJuryTest(run_jury, self.__log) as _test:
            _teams = _test.req_teams()
            i = 0
            try:
                _score = _test.req_scoreboard()
                _expected_sum_act = _score["sum_act"]
                _team_tries = {}
                for _team in _teams:
                    team_id = _team["id"]
                    # remember previous team activity
                    _team_tries[team_id] = _score['scoreboard'][team_id]['tries']

                sended_flags = 0
                send_flags = 5000
                self.__log.info("Trying send flags: %s", str(send_flags))
                while i < send_flags:
                    i = i + 1
                    sended_flags += 1
                    if sended_flags % 250 == 0:
                        print("Sended flags ", sended_flags, "/", send_flags)
                    _expected_sum_act += 1
                    team_id = list(_team_tries)[random.randint(0, len(list(_team_tries))-1)]
                    _team_tries[team_id] += 1
                    _resp = _test.req_flag(team_id, UtilsTests.random_flag())
                    if _resp["code"] not in [403, 200]:
                        _test.err_exit("Shit happen " + str(_resp["code"]))
                self.__log.info("Sended flags: %s", str(sended_flags))
                _score = _test.req_scoreboard()
                if _score["sum_act"] != _expected_sum_act:
                    _test.err_exit(
                        "Wrong sum_act expected " + str(_expected_sum_act) + ", but got " +
                        str(_score["sum_act"])
                    )
                self.__log.info("sum_act %s OK", str(_score["sum_act"]))
                for _team_id, _team_tries in _team_tries.items():
                    _tries = _score['scoreboard'][_team_id]['tries']
                    if _tries != _team_tries:
                        _test.err_exit("Wrong tries for " + _team_id)
                    self.__log.info("%s tries %s OK", team_id, str(_tries))
                self.__log.info(">>>> Everything fine!")
            except requests.exceptions.Timeout:
                _test.err_exit("Timed out on " + str(i))
            # finally:
            #     _test.stop
            #     if run_jury:
            #         UtilsTests.stop_jury(run_jury, pid)

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
        self.__tests[args.test_name](args.run_jury)

        sys.exit(0)
