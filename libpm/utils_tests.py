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

""" Utils for tests """

import os
import sys
import signal
import random
import uuid
import subprocess
import psutil
from .utils_files import UtilsFiles
from .utils_ctf01d_config import UtilsCtf01dConfig


class UtilsTests:
    """ UtilsTests """

    @staticmethod
    def random_flag():
        """ random flag """
        flag = str(uuid.uuid4())
        flag = flag[4:]
        flag = 'c01d' + flag
        flag = flag[:-8]
        flag += str(random.randint(1, 99999999)).rjust(8, '0')
        return flag

    @staticmethod
    def get_root_dir():
        """ get root directory """
        ret = os.path.dirname(os.path.abspath(__file__))
        ret = os.path.join(ret, "..")
        ret = os.path.normpath(ret)
        return ret

    @staticmethod
    def data_test_tmp_path():
        """ data_test_tmp path """
        return os.path.join(UtilsTests.get_root_dir(), "data_test_tmp")

    @staticmethod
    def prepare_data_test_tmp(data_tmp_dir, _log=None):
        """ prepare_data_test_tmp """
        if _log is not None:
            _log.info("Prepare config dir %s", data_tmp_dir)
        if os.path.isdir(data_tmp_dir):
            if _log is not None:
                _log.info("Removing dir %s", data_tmp_dir)
            UtilsFiles.recursive_remove_files(data_tmp_dir)
        os.mkdir(data_tmp_dir)
        return data_tmp_dir

    @staticmethod
    def start_jury(data_tmp_dir):
        """ start_jury """
        _command = ["./ctf01d", "-w", data_tmp_dir, "start"]
        print("Run command: " + " ".join(_command))
        _proc = subprocess.Popen(  # pylint: disable=consider-using-with
            _command,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            shell=False
        )
        _returncode = _proc.poll()
        if _returncode is not None:
            return None
        while _returncode is None:
            _returncode = _proc.poll()
            _ln = _proc.stdout.readline()
            if _ln:
                _ln = _ln.decode("utf-8").strip()
                if "EventLoop started," in _ln:
                    return _proc.pid
                # print(_ln)
        while _ln:
            _ln = _proc.stdout.readline()
            if _ln:
                _ln = _ln.decode("utf-8").strip()
                if "EventLoop started," in _ln:
                    return _proc.pid
                # print(_ln)
            else:
                break
        if _returncode != 0:
            print("ERROR: returncode " + str(_returncode))
            sys.exit(_returncode)
        return None

    @staticmethod
    def stop_any_jury():
        """ stop_any_jury """
        # Source - https://stackoverflow.com/a/5597017
        # Posted by user355252, modified by community. See post 'Timeline' for change history
        # Retrieved 2026-05-27, License - CC BY-SA 3.0
        for proc in psutil.process_iter():
            try:
                # Check if process name contains the given name string.
                if "ctf01d" in proc.name().lower():
                    proc.send_signal(signal.SIGKILL)
            except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
                pass

    @staticmethod
    def stop_jury(pid):
        """ stop_jury """
        os.kill(pid, signal.SIGKILL)

    @staticmethod
    def start_empty_jury_1x3(_log):
        """ start_empty_jury_1x3 """
        _cfg_dir = UtilsTests.data_test_tmp_path()
        UtilsTests.prepare_data_test_tmp(_cfg_dir, _log)

        # _cfg = UtilsCtf01dConfig.generate_default(count_teams=3, count_services=1)
        # UtilsCtf01dConfig.write_to_file(_cfg_dir, _cfg)
        UtilsCtf01dConfig.write_example_config_1x3(_cfg_dir)
        UtilsCtf01dConfig.write_checker_test(_cfg_dir, "test_service1")

        UtilsTests.stop_any_jury()
        pid = UtilsTests.start_jury(_cfg_dir)
        if pid is None:
            UtilsTests.print_error_and_exit(_log, "Could not run ctf01d (!!!!!)")
        return _cfg_dir, pid

    @staticmethod
    def print_error_and_exit(_log, msg):
        """ print_error_and_exit """
        _log.error(
            "\n"
            "\n************************************************"
            "\n* %s *"
            "\n************************************************"
            "\n",
            msg
        )
        UtilsTests.stop_any_jury()
        sys.exit(1)
