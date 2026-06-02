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

""" rebuild images """

import os
import sys
import logging
import datetime
from .utils_shell import UtilsShell
from .utils_log import UtilsLog
from .pm_config import PmConfig


class CommandRebuildEnvironmentImages:
    """ CommandRebuildEnvironmentImages """

    def __init__(self, config: PmConfig):
        self.__log = UtilsLog("CommandRebuildEnvironmentImages").get_logger()
        self.__log.setLevel(logging.DEBUG)
        self.__config = config
        now = datetime.datetime.now()
        self.__dt_tag = now.strftime("%Y-%m-%d")
        self.__subcommand_name = "rebuild-environment-images"
        self.__debian_version = "13"
        self.__base_tag = "sea5kg/ctf01d"
        self.__packages = {
            "build": [
                "make",
                "cmake",
                "gcc",
                "g++",
                "curl",
                "pkg-config",
                "libcurl4-openssl-dev",
                "zlib1g-dev",
                "libpng-dev",
                "python3",
                "python3-pip",
                "apt-utils",
            ],
            "build-python": [
                "requests",
                "faker",
                "pylint",
                "pycodestyle",
            ],
            "release": [
                "libcurl4",
                "zlib1g",
                "libpng16-16",
                "libpthread-stubs0-dev",
                "libssl-dev",
                "locales",
                "nano",
                "vim",
                "python3",
                "python3-pip",
                "python-is-python3",
                "telnet",
                "iputils-ping",
            ],
            "release-python": [
                "requests",
                "faker",
                "grpcio",
                "grpcio-tools",
                "protobuf",
                "tzdata",
                "bs4",
                "mimesis",
            ]
        }

    def get_name(self):
        """ return subcommand name """
        return self.__subcommand_name

    def do_registry(self, subparsers):
        """ registering sub command """
        desc = "Rebuild environment images"
        desc += " (Dockerfile.build-environment && Dockerfile.release-environment)"
        _parser_rebuild_env_images = subparsers.add_parser(
            name=self.__subcommand_name,
            description=desc
        )
        _parser_rebuild_env_images.set_defaults(subparser=self.__subcommand_name)

    def __build_docker_image(self, tag, filename):
        cmd = "docker build --rm --tag " + tag + " -f " + filename + " ."
        ret = os.system(cmd)
        if ret != 0:
            self.__log.error("ERROR: Could not build image by command: %s", cmd)
            sys.exit(1)

    def __has_image(self, full_tag):
        exit_code, output = UtilsShell.run_command_get_output(self.__log, [
            "docker", "images", full_tag, "--format", "{{json . }}"
        ])
        if exit_code != 0:
            # self.__log.info("exit_code: %s", exit_code)
            self.__log.error("Could not execute command 'docker images...', output %s", output)
            sys.exit(-1)
        # self.__log.info("output: %s", output)
        return output != ""  # output not empty... so has image

    def __update_dockerfile_build_env(self):
        _filename = "Dockerfile.build-environment"
        self.__log.info("Update file %s", _filename)
        os.chdir(self.__config.get_root_dir())

        with open(_filename, "wt", encoding="utf-8", newline="\n") as _file:
            _file.write("""FROM debian:""" + self.__debian_version + """
WORKDIR /root/

LABEL "maintainer"="Evgenii Sopov <mrseakg@gmail.com>"
LABEL "repository"="https://github.com/sea5kg/ctf01d"

RUN apt-get -y update \\
  && apt-get -y upgrade \\
  && apt-get install -y --no-install-recommends \\
    """ + " \\\n    ".join(self.__packages["build"]) + """ \\
  && pip3 install --break-system-packages \\
    """ + " \\\n    ".join(self.__packages["build-python"]) + """ \\
  && apt-get clean
""")
        return _filename

    def __update_dockerfile_release_env(self):
        _filename = "Dockerfile.release-environment"
        self.__log.info("Update file %s", _filename)
        os.chdir(self.__config.get_root_dir())
        with open(_filename, "wt", encoding="utf-8", newline="\n") as _file:
            _file.write("""FROM debian:""" + self.__debian_version + """

LABEL "maintainer"="Evgenii Sopov <mrseakg@gmail.com>"
LABEL "repository"="https://github.com/sea5kg/ctf01d"

RUN apt-get -y update \\
  && apt-get -y upgrade \\
  && apt-get install -y --no-install-recommends \\
    """ + " \\\n    ".join(self.__packages["release"]) + """ \\
  && pip3 install --break-system-packages \\
    """ + " \\\n    ".join(self.__packages["release-python"]) + """ \\
  && apt-get clean

# RUN locale-gen en_US.UTF-8
RUN sed -i -e "s/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/" /etc/locale.gen && \\
    echo 'LANG="en_US.UTF-8"'>/etc/default/locale && \\
    dpkg-reconfigure --frontend=noninteractive locales && \\
    update-locale LANG=en_US.UTF-8
# RUN update-locale LANG=en_US.UTF-8
""")
        return _filename

    def __silent_remove_image(self, image_tag):
        if self.__has_image(image_tag):
            self.__log.info("Found image %s, try removing...", image_tag)
            ret = os.system("docker rmi " + image_tag)
            if ret != 0:
                self.__log.error("Could not remove %s", image_tag)
                sys.exit(1)

    def execute(self, _):
        """ executing """
        os.chdir(self.__config.get_root_dir())
        build_env = self.__update_dockerfile_build_env()
        release_env = self.__update_dockerfile_release_env()
        self.__log.info("Rebuild environment images...")

        tag_build = self.__base_tag
        tag_build_today = tag_build + ":build-environment-" + self.__dt_tag
        tag_build_latest = tag_build + ":build-environment-latest"
        tag_release = self.__base_tag
        tag_release_today = tag_release + ":release-environment-" + self.__dt_tag
        tag_release_latest = tag_release + ":release-environment-latest"

        self.__silent_remove_image(tag_build_today)
        self.__build_docker_image(tag_build_today, build_env)

        self.__silent_remove_image(tag_build_latest)
        self.__build_docker_image(tag_build_latest, build_env)

        self.__silent_remove_image(tag_release_today)
        self.__build_docker_image(tag_release_today, release_env)

        self.__silent_remove_image(tag_release_latest)
        self.__build_docker_image(tag_release_latest, release_env)
        sys.exit(0)
