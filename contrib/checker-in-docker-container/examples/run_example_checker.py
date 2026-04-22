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

"""
    Example run checker in multithreading
"""

import os
import threading
import random
import string
import copy
import subprocess
import json

def task(_host_mask, _host_x, _flags, _timeout, _output_json):
    service_status = {
        101: 'u',  # up - the flag putting/checking into the service is successful
        102: 'c',  # corrupt - service is available (available tcp connection) but it's impossible to put/get the flag
        103: 'm',  # mumble - checker-script worked long time than allowed (this state will be set by ctf01d)
        104: 'd',  # down - service is not available (maybe blocked port or service is down)
        # *: 's',  # problems in checker (this state will be set by ctf01d), for checker developers
    }
    all_ret = []

    # print(f"put flag '{_flag_id}' '{_flag_value}' to {_host}")
    for _flag_id, _flag_value in _flags.items():
        command = ["python3", "-u", "example_checker.py"]
        command.extend([
            _host_mask[0] + _host_x + _host_mask[1],
            "put",
            _flag_id,
            _flag_value
        ])
        try:
            result = subprocess.run(command, capture_output=True, text=True, timeout=_timeout)
            all_ret.append(result.returncode)
            if result.returncode not in service_status:
                print(result.stdout)
            # this output developers can be look in checker-as-services log
            # result.stdout
        except subprocess.TimeoutExpired:
            _output_json[_host_x] = 'm'  # mumble
            return

        # print(result.stdout)  # The command's output
    ret = all_ret[0]
    if ret in service_status:
        _output_json[_host_x] = service_status[ret]
    else:
        _output_json[_host_x] = 's'  # problems in checker, for checker developers
    # print(f"ret = {ret}")

def random_flag_value():
    # c01d01b2-3c0e-2089-ccdb-19ac75853209
    hex_chars = "abcdef1234567890"
    ret = "c01d"
    ret += ''.join(random.choices(hex_chars, k=4))
    for i in range(0,4):
        ret += "-" + ''.join(random.choices(hex_chars, k=4))
    ret += ''.join(random.choices("1234567890", k=8))  # seconds from game start
    return ret

flag_id_chars = string.ascii_letters + string.digits

threads = []

example_input_json = {
    "t": 5,  # timeout in seconds
    "m": ["service", ""],  # prefix and suffix of host name, for example: ["10.22.", ".3"]
}

i = 0
max_services = 25
while i < max_services:
    i += 1
    if i == 24:
        continue
    _flag_id = "run_test_" + ''.join(random.choices(flag_id_chars, k=10))
    _host_x = str(i)
    example_input_json[_host_x] = {
        _flag_id: random_flag_value(),
    }

print("input:", str(example_input_json).replace("'", "\""))

example_output_json = {}
_timeout = example_input_json["t"]
_host_mask = example_input_json["m"]
for _host_x, _flags in example_input_json.items():
    if _host_x in ["t", "m"]:  # skip timeout, prefix host option
        continue
    # x - some problem with a checker-as-service
    example_output_json[_host_x] = {'s':'x'}
    threads.append(threading.Thread(target=task, args=(
        _host_mask, _host_x,
        copy.deepcopy(_flags),
        _timeout,
        example_output_json
    )))

for _thread in threads:
    _thread.start()

for _thread in threads:
    _thread.join()

print("output:", str(example_output_json).replace("'", "\""))

print("All request completed")