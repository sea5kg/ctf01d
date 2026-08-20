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

""" Command for linting python scripts """

import sys
import os
import logging
from .utils_shell import UtilsShell
# from .pm_config import PmConfig

logging.basicConfig()


class CommandPyCheck:
    """ CommandPyCheck """
    def __init__(self, _):  # config: PmConfig):
        self.__log = logging.getLogger("CommandPyCheck")
        self.__log.setLevel(logging.DEBUG)
        # self.__config = config
        self.__subcommand_name = "py-check"

    def get_name(self):
        """ return subcommand name """
        return self.__subcommand_name

    def do_registry(self, subparsers):
        """ registering sub command """
        _parser = subparsers.add_parser(
            name=self.__subcommand_name,
            description='Check python files'
        )
        _parser.set_defaults(subparser=self.__subcommand_name)

    def execute(self, _):
        """ executing """
        self.__log.info("Starting py-check...")
        check_python_files = [
            "libpm",
        ]

        for _file in os.listdir("libpm/data"):
            if _file.endswith(".py"):
                check_python_files.append("libpm/data/" + _file)

        failed = []
        for py_file in check_python_files:
            ret, _output = UtilsShell.run_command_get_output(
                self.__log,
                ['python3', '-m', 'pycodestyle', py_file, '--max-line-length=100'],
            )
            if ret == 0:
                self.__log.info("OK (pep8): %s", py_file)
            else:
                failed.append(py_file)
                self.__log.error(_output)
                self.__log.info("FAIL (pep8): %s", py_file)

            ret, _output = UtilsShell.run_command_get_output(
                self.__log,
                ['python3', '-m', 'pylint', py_file, '--max-line-length=100'],
            )
            if ret == 0:
                self.__log.info("OK (pylint): %s", py_file)
            else:
                failed.append(py_file)
                self.__log.error(_output)
                self.__log.info("FAIL (pylint): %s", py_file)

        if len(failed) > 0:
            self.__log.error("Has fails")
            sys.exit(1)
        self.__log.info("Looks fine.")
        sys.exit(0)
