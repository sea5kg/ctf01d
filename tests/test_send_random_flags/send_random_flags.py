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

import requests
import sys
import time
import uuid
import random

host = "localhost"
port = 8080
base_url = 'http://' + host + ':' + str(port) + ''
debug = False
start_time = time.time()
sended_flags_in_period = 0
print("Sending random flags by random team")

team_ids = []

http_get_url = base_url + '/api/v1/game'
# print("Request " + http_get_url)
r = requests.get(http_get_url)
game_info = r.json()
for team in game_info['teams']:
    team_ids.append(team['id'])

print("Nnmber of Teams:", len(team_ids))

i = 0
while i < 1000000000:
    i = i + 1
    sended_flags_in_period += 1
    teamid = team_ids[random.randint(0, len(team_ids)-1)]

    # print(teamid)
    flag = str(uuid.uuid4())
    flag = flag[4:]
    flag = 'c01d' + flag
    flag = flag[:-8]
    flag += str(random.randint(1, 99999999)).rjust(8, '0')

    http_get_url = base_url + '/flag?teamid=' + str(teamid) + '&flag=' + flag
    # print("Request " + http_get_url)
    r = requests.get(http_get_url)
    if r.status_code == 200:
        if debug:
            print("OK")
    elif r.status_code == 400:
        if debug:
            print("FAIL " + r.text + "\n" + flag)
        sys.exit(1)
    elif r.status_code == 403:
        if debug:
            print("FAIL " + flag + " " + r.text)
    else:
        print("Wrong params " + str(r.status_code))
    # time.sleep(0.05)
    if i % 100 == 0 and sended_flags_in_period > 0:
        end_time = time.time()
        elapsed_time = end_time - start_time
        speed = sended_flags_in_period / elapsed_time
        print("%.2f" % speed, "flags per second")
        start_time = time.time()
        sended_flags_in_period = 0
