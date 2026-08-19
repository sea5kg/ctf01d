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

""" Helpers for processing docker """

import sys
import os
from .utils_shell import UtilsShell


class UtilsDocker:
    """ UtilsDocker """

    @staticmethod
    def has_image(full_tag, _log):
        """ check image exists """
        exit_code, output = UtilsShell.run_command_get_output(_log, [
            "docker", "images", full_tag, "--format", "{{json . }}"
        ])
        if exit_code != 0:
            # self.__log.info("exit_code: %s", exit_code)
            _log.error("Could not execute command 'docker images...', output %s", output)
            sys.exit(-1)
        # self.__log.info("output: %s", output)
        return output != ""  # output not empty... so has image

    @staticmethod
    def silent_remove_image(image_tag, _log):
        """ remove image if exists """
        if UtilsDocker.has_image(image_tag, _log):
            _log.info("Found image %s, try removing...", image_tag)
            ret = os.system("docker rmi " + image_tag)
            if ret != 0:
                _log.error("Could not remove %s", image_tag)
                sys.exit(1)

    @staticmethod
    def build_docker_image(tag, filename, _log):
        """ build image """
        cmd = "docker build --rm --tag " + tag + " -f " + filename + " ."
        ret = os.system(cmd)
        if ret != 0:
            _log.error("ERROR: Could not build image by command: %s", cmd)
            sys.exit(1)
