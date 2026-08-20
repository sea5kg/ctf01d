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

"""
    checker for vuln_example_service.py
"""

import sys
import socket
import traceback
import errno


def service_up():
    """
    put-get flag to service success
    """
    print("[service is worked] - 101")
    sys.exit(101)


def service_corrupt():
    """
    service is available (available tcp connect) but protocol wrong could not put/get flag
    """
    print("[service is corrupt] - 102")
    sys.exit(102)


def service_mumble():
    """
    waited time (for example: 5 sec) but service did not have time to reply
    """
    print("[service is mumble] - 103")
    sys.exit(103)


def service_down():
    """
    service is not available (maybe blocked port or service is down)
    """
    print("[service is down] - 104")
    sys.exit(104)


if len(sys.argv) != 5:
    _FLAG_ID = "abcdifghr"
    _FLAG_VALUE = "123e4567-e89b-12d3-a456-426655440000"
    print(
        "\n"
        "Usage:\n"
        "\t" + sys.argv[0] + " <host> (put|check) <flag_id> <flag>\n"
        "Example:\n"
        "\t" + sys.argv[0] + " \"127.0.0.1\" put \"" + _FLAG_ID + "\" \"" + _FLAG_VALUE + "\" \n"
        "\n"
    )
    sys.exit(-1)

DEBUG_ENABLED = False


def debug(err):
    """ debug """
    if DEBUG_ENABLED:
        if isinstance(err, str):
            err = Exception(err)
        traceback.print_exc()
        raise err


class Config:  # pylint: disable=too-few-public-methods
    """ Class config with host, port, flag and etc."""
    def __init__(self):
        self.host = sys.argv[1]
        self.port = 4101
        self.command = sys.argv[2]
        self.f_id = sys.argv[3]
        self.flag = sys.argv[4]


CONFIG = Config()

# will be mumble (2) - for test jury
# while True: time.sleep(10);


def put_flag(cfg: Config):
    """ put flag on service """
    # try put
    try:
        # print("try connect " + cfg.host + ":" + str(cfg.port))
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(1)
        s.connect((cfg.host, cfg.port))
        s.recv(1024)
        s.send(("put" + "\n").encode("utf-8"))
        s.recv(1024)
        s.send((cfg.f_id + "\n").encode("utf-8"))
        s.recv(1024)
        s.send((cfg.flag + "\n").encode("utf-8"))
        s.recv(1024)
        s.close()
    except socket.timeout:
        service_down()
    except socket.error as _err:
        if _err.errno == errno.ECONNREFUSED:
            service_down()
        else:
            debug(_err)
            service_corrupt()
    except Exception as e:  # pylint: disable=broad-exception-caught
        debug(e)
        service_corrupt()


def check_flag(cfg: Config):
    """ check flag """
    flag2 = ""
    try:
        # print("try connect " + cfg.host + ":" + str(cfg.port))
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(1)
        s.connect((cfg.host, cfg.port))
        s.recv(1024)
        s.send(("get\n").encode("utf-8"))
        s.recv(1024)
        s.send((cfg.f_id + "\n").encode("utf-8"))
        result = s.recv(1024)
        result = result.decode("utf-8", "ignore")
        flag2 = result.strip()
        flag2 = flag2.split("FOUND FLAG: ")
        if len(flag2) == 2:
            flag2 = flag2[1]
        else:
            flag2 = ''
        s.close()
    except socket.timeout:
        service_down()
    except socket.error as _err:
        if _err.errno == errno.ECONNREFUSED:
            service_down()
        else:
            debug(_err)
            service_corrupt()
    except Exception as e:  # pylint: disable=broad-exception-caught
        debug(e)
        service_corrupt()

    if cfg.flag != flag2:
        debug('flag: [' + cfg.flag + '] flag2: [' + str(flag2) + ']')
        service_corrupt()


if CONFIG.command == "put":
    put_flag(CONFIG)
    check_flag(CONFIG)
    service_up()

if CONFIG.command == "check":
    check_flag(CONFIG)
    service_up()
