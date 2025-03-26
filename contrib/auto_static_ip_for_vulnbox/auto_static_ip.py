#!/usr/bin/env python3

"""
    Script for automaticly detect ip address and set static.
    This script can be helping for auto set static ip to vulnbox.
    Based for debian.
"""

import socket
import sys
import os
import time
import platform
import subprocess
import requests


def get_myip():
    """ try get my ip from jury """
    print("> Trying get my ip from jury...")
    ret = None
    jury_hosts = [
        "10.10.100.100:8080",
        "10.10.100.100:80",
        "192.168.88.252:8080",
    ]
    for _juryhost in jury_hosts:
        try:
            _jury_url = "http://" + _juryhost + "/api/v1/myip"
            print("  * Try ", _jury_url)
            resp = requests.get(_jury_url, timeout=3)
            ret = resp.json()['myip']
            break
        except Exception as _err:  # pylint: disable=broad-except
            print("  * Error:", str(_err))
            continue
    print("return", ret)
    if ret is None:
        print("FAILED: cound not get myip from jury")
        # sys.exit(-1)
    return ret


def detect_current_ip():
    """ detect current ip """
    print("> Detecting current ip via socket...")
    _test_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    _test_socket.connect(("10.10.100.100", 8080))
    ret = _test_socket.getsockname()[0]
    _test_socket.close()
    print("return", ret)
    return ret


def ping(host):
    """ ping """
    print("> Ping host:", host)
    parameter = '-n' if platform.system().lower() == 'windows' else '-c'
    command = ['ping', parameter, '1', host]
    ret = subprocess.run(command, stdout=subprocess.PIPE, check=False)
    ret = ret.returncode == 0
    print("return", ret)
    return ret


def copyfile(_filepath_in, _filepath_out):
    """ copyfile """
    _content = ""
    print("> Copy file from", _filepath_in, "to", _filepath_out)
    with open(_filepath_in, 'rt', encoding="utf-8") as _file:
        _content = _file.read()
    with open(_filepath_out, 'wt', encoding="utf-8") as _file:
        _file.write(_content)


def get_current_ip_wait():
    """ get_current_ip_wait """
    print("> Getting local ip...")
    ret = None
    while ret is None:
        try:
            print("  * Try getting local ip...")
            ret = detect_current_ip()
        except OSError as _err:  # pylint: disable=bare-except
            print("  * Error: ", _err)
            time.sleep(5)
    print("return", ret)
    return ret


def restart_networking():
    """ restart networking """
    print("> Restarting networking...")
    os.system("systemctl restart networking")
    print("> Wait 5 sec when netwoking apply...")
    time.sleep(5)


INTERFACES_PATH = "/etc/network/interfaces"
INTERFACES_BACKUP_PATH = "/etc/network/interfaces.backup"


print("> Wait 15 seconds before start")
time.sleep(15)

MY_IP = get_myip()
if MY_IP is None:
    print("> Could not available jury machine. Try reset interfaces to backup")
    if not os.path.isfile(INTERFACES_BACKUP_PATH):
        print("FAILED: Could not find file", INTERFACES_BACKUP_PATH)
        sys.exit(-1)
    copyfile(INTERFACES_BACKUP_PATH, INTERFACES_PATH)
    restart_networking()

MY_IP = get_myip()
CURRENT_IP = get_current_ip_wait()

while CURRENT_IP != MY_IP:
    print(
        "> Try reset ip config (because ip different", CURRENT_IP, "!=", MY_IP
    )
    if not os.path.isfile(INTERFACES_BACKUP_PATH):
        print("FAILED: Could not find file", INTERFACES_BACKUP_PATH)
        sys.exit(-1)

    print("> Try restoring network config...")
    copyfile(INTERFACES_BACKUP_PATH, INTERFACES_PATH)
    restart_networking()

    MY_IP = get_myip()
    CURRENT_IP = get_current_ip_wait()

print("> Equals IPs. Nice.")

if CURRENT_IP.endswith(".3"):
    print("> Looks fine IP for VULNBOX", CURRENT_IP)
    sys.exit(0)

VULNBOX_IP = CURRENT_IP.split(".")
VULNBOX_IP[3] = '3'
VULNBOX_IP = ".".join(VULNBOX_IP)

if ping(VULNBOX_IP):
    print('> VULNBOX found in current subnetwork. Stop')
    sys.exit(0)

VULNBOX_GATEWAY = CURRENT_IP.split(".")
VULNBOX_GATEWAY[3] = '1'
VULNBOX_GATEWAY = ".".join(VULNBOX_GATEWAY)

print('> VULNBOX not found. Trying set static IP address....')

DEFAULT_INTERFACES_WITH_STATIC_IP = """
# This file describes the network interfaces available on your system
# and how to activate them. For more information, see interfaces(5).

source /etc/network/interfaces.d/*

# The loopback network interface
auto lo
iface lo inet loopback

# The primary network interface
allow-hotplug enp0s3
# iface enp0s3 inet dhcp
iface enp0s3 inet static
address """ + VULNBOX_IP + """
netmask 255.255.255.0
gateway """ + VULNBOX_GATEWAY + """
"""

if not os.path.isfile(INTERFACES_BACKUP_PATH):
    print("> Making backup", INTERFACES_PATH, "->", INTERFACES_BACKUP_PATH)
    copyfile(INTERFACES_PATH, INTERFACES_BACKUP_PATH)

with open(INTERFACES_PATH, 'wt', encoding="utf-8") as _file:
    print("> Override file", INTERFACES_PATH)
    _file.write(DEFAULT_INTERFACES_WITH_STATIC_IP)

restart_networking()

MY_IP = get_myip()
CURRENT_IP = get_current_ip_wait()

if CURRENT_IP == MY_IP:
    print("> Looks everything done... Have a nice game!")
    sys.exit(0)

print("> Oops. Something went wrong. Trying reset ip config from backup")
if not os.path.isfile(INTERFACES_BACKUP_PATH):
    print("> Could not find file:", INTERFACES_BACKUP_PATH)
    sys.exit(-1)

copyfile(INTERFACES_BACKUP_PATH, INTERFACES_PATH)
restart_networking()
print("> Restored")
