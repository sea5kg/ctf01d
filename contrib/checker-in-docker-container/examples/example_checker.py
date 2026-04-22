#!/usr/bin/env python3
#  _______ _________ _______  _______  __    ______
# (  ____ \\__   __/(  ____ \(  __   )/  \  (  __  \
# | (    \/   ) (   | (    \/| (  )  |\/) ) | (  \  )
# | |         | |   | (__    | | /   |  | | | |   ) |
# | |         | |   |  __)   | (/ /) |  | | | |   | |
# | |         | |   | (      |   / | |  | | | |   ) |
# | (____/\   | |   | )      |  (__) |__) (_| (__/  )
# (_______/   )_(   |/       (_______)\____/(______/
#
# MIT License
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

import sys
import math
import socket
import random
import time
import errno

# put-get flag to service success
def service_up():
    print("[service is worked] - 101")
    sys.exit(101)

# service is available (available tcp connect) but protocol wrong could not put/get flag
def service_corrupt():
    print("[service is corrupt] - 102")
    sys.exit(102)

# waited time (for example: 5 sec) but service did not have time to reply
def service_mumble():
    print("[service is mumble] - 103")
    sys.exit(103)

# service is not available (maybe blocked port or service is down)
def service_down():
    print("[service is down] - 104")
    sys.exit(104)

if len(sys.argv) != 5:
    print("\nUsage:\n\t" + sys.argv[0] + " <host> (put|check) <flag_id> <flag>\n")
    print("Example:\n\t" + sys.argv[0] + " \"127.0.0.1\" put \"abcdifghr\" \"c01d4567-e89b-12d3-a456-426655440000\" \n")
    print("\n")
    sys.exit(0)

host = sys.argv[1]
port = 4101
command = sys.argv[2]
f_id = sys.argv[3]
flag = sys.argv[4]

# will be mumble (2) - for test jury
# while True: time.sleep(10);

def put_flag():
    print("put_flag")
    global host, port, f_id, flag
    # try put
    try:
        # print("try connect " + host + ":" + str(port))
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(1)
        s.connect((host, port))
        _ = s.recv(1024).decode("utf-8")
        # print(result)
        s.send("put\n".encode())
        _ = s.recv(1024).decode("utf-8")
        s.send(str(f_id + "\n").encode())
        _ = s.recv(1024).decode("utf-8")
        s.send(str(flag + "\n").encode())
        _ = s.recv(1024).decode("utf-8")
        s.close()
    except socket.timeout:
        service_mumble()
    except socket.error as serr:
        if serr.errno == errno.ECONNREFUSED:
            service_down()
        else:
            print(serr)
            service_corrupt()
    except Exception as e:
        print(e)
        service_corrupt()

def check_flag():
    print("check_flag")
    global host, port, f_id, flag
    # try get
    flag2 = ""
    try:
        # print("try connect " + host + ":" + str(port))
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(10)
        s.connect((host, port))
        result = s.recv(1024).decode("utf-8")
        # print(result)
        s.send("get\n".encode())
        result = s.recv(1024).decode("utf-8")
        s.send(str(f_id + "\n").encode())
        result = s.recv(1024).decode("utf-8")
        flag2 = result.strip()
        flag2 = flag2.split("FOUND FLAG: ")
        if len(flag2) == 2:
            flag2 = flag2[1]
        else:
            flag2 = ''
        s.close()
    except socket.timeout:
        service_down()
    except socket.error as serr:
        if serr.errno == errno.ECONNREFUSED:
            service_down()
        else:
            print(serr)
            service_corrupt()
    except Exception as e:
        print(e)
        service_corrupt()

    if flag != flag2:
        service_corrupt()


if command == "put":
    put_flag()
    check_flag()
    service_up()

if command == "check":
    check_flag()
    service_up()