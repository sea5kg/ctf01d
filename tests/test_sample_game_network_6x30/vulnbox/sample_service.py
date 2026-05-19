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
Sample service for testing ctf01d
"""

import socket
import threading
import re
import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
FLAGS_DIR = os.path.join(SCRIPT_DIR, 'flags')

if len(sys.argv) < 2:
    sys.exit("Expeceted parameter <port>")

PORT = int(sys.argv[1])  # 4101
thrs = []


class Connect(threading.Thread):
    """
        Class for handling connects
    """
    def __init__(self, sock, addr):
        self.__sock = sock
        self.__addr = addr
        self.__is_kill = False
        self.__commands = {
            "put": self.__handle_command_put,
            "get": self.__handle_command_get,
            "delete": self.__handle_command_delete,
            "list": self.__handle_command_list,
            "close": self.__handle_command_close,
        }
        self.__welcome = "\n"
        self.__welcome += "Welcome to sample_service\n"
        self.__welcome += "Commands: put, get, delete, list, close\n"
        self.__welcome += "> "
        threading.Thread.__init__(self)

    def __handle_command_list(self):
        counter = 0
        for filename in os.listdir(FLAGS_DIR):
            counter += 1
            resp = "file: " + filename + "\n"
            self.__sock.send(resp.encode())
        if counter == 0:
            self.__sock.send("*nothing*".encode())

    def __handle_command_close(self):
        resp = "\nBye-bye\n\n"
        self.__sock.send(resp.encode())

    def __handle_command_put(self):
        resp = "flag_id = "
        self.__sock.send(resp.encode())
        f_id = self.__sock.recv(1024)
        f_id = f_id.decode("utf-8", "ignore")
        f_id = f_id.strip()
        if f_id == "":
            return
        orig_flag_id = f_id
        f_id = re.search(r"\w*", f_id).group()
        if f_id == "" or f_id != orig_flag_id:
            resp = "\nFAIL: Incorrect flag_id\n"
            self.__sock.send(resp.encode())
            return
        resp = "flag = "
        self.__sock.send(resp.encode())
        f_text = self.__sock.recv(1024)
        f_text = f_text.decode("utf-8", "ignore")
        if f_text == "":
            return
        with open(os.path.join(FLAGS_DIR, f_id), 'w') as _file:
            _file.write(f_text)
        self.__sock.send("OK".encode())

    def __handle_command_get(self):
        resp = "flag_id = "
        self.__sock.send(resp.encode())
        f_id = self.__sock.recv(1024)
        f_id = f_id.decode("utf-8", "ignore")
        f_id = f_id.strip()
        if f_id == "":
            return
        orig_flag_id = f_id
        f_id = re.search(r"\w*", f_id).group()
        if f_id == "" or f_id != orig_flag_id:
            resp = "\nFAIL: Incorrect flag_id\n"
            self.__sock.send(resp.encode())
            return
        if os.path.exists(os.path.join(FLAGS_DIR, f_id)):
            with open(os.path.join(FLAGS_DIR, f_id), 'r') as _file:
                line = _file.readline()
            resp = "FOUND FLAG: " + line + ""
            self.__sock.send(resp.encode())
        else:
            resp = "\nFAIL: flag_id not found\n"
            self.__sock.send(resp.encode())

    def __handle_command_delete(self):
        resp = "flag_id = "
        self.__sock.send(resp.encode())
        f_id = self.__sock.recv(1024)
        f_id = f_id.decode("utf-8", "ignore")
        f_id = f_id.strip()
        if f_id == "":
            return
        orig_flag_id = f_id
        f_id = re.search(r"\w*", f_id).group()
        if f_id == "" or f_id != orig_flag_id:
            resp = "\nFAIL: Incorrect flag_id\n"
            self.__sock.send(resp.encode())
            return
        if os.path.exists(os.path.join(FLAGS_DIR, f_id)):
            os.remove(os.path.join(FLAGS_DIR, f_id))
            resp = "REMOVED"
            self.__sock.send(resp.encode())
        else:
            resp = "\nFAIL: flag_id not found\n"
            self.__sock.send(resp.encode())

    def run(self):
        self.__sock.send(self.__welcome.encode())
        while True:
            if self.__is_kill is True:
                break
            buf = self.__sock.recv(1024)
            buf = buf.decode("utf-8", "ignore")
            buf = buf.strip()
            if buf == "":
                break
            command = re.search(r"\w*", buf).group()
            if command in self.__commands:
                self.__commands[command]()
                break
            if command:
                resp = "\n [" + command + "] unknown command\n\n"
                self.__sock.send(resp.encode())
                break
        self.__is_kill = True
        # it's will be corrapt service
        # self.__sock.send("bye!\n".encode())
        self.__sock.close()
        thrs.remove(self)

    def kill(self):
        """ kill thread client """
        if self.__is_kill is True:
            return
        self.__is_kill = True
        self.__sock.close()
        # thrs.remove(self)


SERVER_SOCKET = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
SERVER_SOCKET.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
SERVER_SOCKET.bind(('0.0.0.0', PORT))
SERVER_SOCKET.listen(10)

print("Listen in port 0.0.0.0:" + str(PORT))

if not os.path.exists("flags"):
    os.makedirs("flags")

try:
    while True:
        sock_client, addr_client = SERVER_SOCKET.accept()
        thr = Connect(sock_client, addr_client)
        thrs.append(thr)
        thr.start()
except KeyboardInterrupt:
    print('Bye! Write me letters!')
    SERVER_SOCKET.close()
    for thr in thrs:
        thr.kill()
