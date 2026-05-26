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

import sys
import os
import argparse
import libpm

ROOT_DIR = os.path.dirname(os.path.realpath(__file__))
# find the root dir (dir which contains dir 'libpm')
PM_DIR = os.path.join(ROOT_DIR, 'libpm')
PM_FILE = os.path.join(ROOT_DIR, 'pm.py')
while not os.path.isdir(PM_DIR) and not os.path.isfile(PM_FILE):
    ROOT_DIR = os.path.join(ROOT_DIR, '..')
    ROOT_DIR = os.path.normpath(ROOT_DIR)
    PM_DIR = os.path.join(ROOT_DIR, 'libpm')
    PM_FILE = os.path.join(ROOT_DIR, 'pm.py')
    if ROOT_DIR == "/":
        sys.exit("Could not find rood dir")
# print("ROOT_DIR =", ROOT_DIR)

libpm.run_main(ROOT_DIR)

CONFIG = libpm.PmConfig(ROOT_DIR)

COMMANDS = [
    libpm.CommandClean(CONFIG),
    libpm.CommandCodeStats(CONFIG),
    libpm.CommandPyCheck(CONFIG),
    libpm.CommandCheck(CONFIG),
    libpm.CommandClangFormat(CONFIG),
    libpm.CommandRebuildEnvironmentImages(CONFIG),
    libpm.CommandTests(CONFIG)
]

class CustomActionHelp(argparse._HelpAction):  # pylint: disable=protected-access
    """ custom help action """
    def __call__(self, parser, namespace, values, option_string=None):
        libpm.print_custom_help(parser)


if __name__ == "__main__":
    MAIN_PARSER = argparse.ArgumentParser(
        prog='pm',
        description='Project manager for current project',
        epilog='Helper tools for work with current project',
        add_help=False
    )
    MAIN_PARSER.add_argument('--help', '-h', action=CustomActionHelp, help='help')

    SUBCOMMANDS = MAIN_PARSER.add_subparsers(title='subcommands')
    for _command in COMMANDS:
        _command.do_registry(SUBCOMMANDS)
    ARGS = MAIN_PARSER.parse_args()
    if 'subparser' not in ARGS:
        libpm.print_custom_help(MAIN_PARSER)
        sys.exit(1)
    SUBCOMMAND = ARGS.subparser
    for _command in COMMANDS:
        if _command.get_name() == SUBCOMMAND:
            _command.execute(ARGS)
