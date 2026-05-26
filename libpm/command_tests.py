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
import requests
from .utils_files import UtilsFiles
from .utils_log import UtilsLog
from .pm_config import PmConfig


class CommandTests:
    """ CommandTests """
    def __init__(self, config: PmConfig):
        self.__log = UtilsLog("CommandTests").get_logger()
        self.__config = config
        self.__subcommand_name = "tests"
        self.__tests = {
            "path_traversal": self.__test_path_traversal
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

    def __cleanup_data_tmp(self):
        root_dir = self.__config.get_root_dir()
        data_tmp_dir = os.path.join(root_dir, "data_tmp")
        if os.path.isdir(data_tmp_dir):
            self.__log.info("Removing dir %s", data_tmp_dir)
            UtilsFiles.recursive_remove_files(data_tmp_dir)
        os.mkdir(data_tmp_dir)
        return data_tmp_dir

    def __test_path_traversal(self):
        self.__log.info("Run test 'Path Traversal'.")
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
        self.__log.info("OK")

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
        self.__cleanup_data_tmp()
        self.__tests[args.test_name]()

        sys.exit(0)
