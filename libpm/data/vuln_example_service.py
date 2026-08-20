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
Sample vuln-service for testing ctf01d
"""

import socket
import sqlite3
import threading
import re
import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
FLAGS_DIR = os.path.join(SCRIPT_DIR, 'flags')
FLAGS_DB = os.path.join(FLAGS_DIR, 'flags.db')

if len(sys.argv) < 2:
    sys.exit("Expected parameter <port>")

PORT = int(sys.argv[1])  # 4101
CLIENT_THREADS = []

# Allow sharing, but it is NOT inherently thread-safe yet
SHARED_CONN = sqlite3.connect(FLAGS_DB, check_same_thread=False)
DB_LOCK = threading.Lock()

with DB_LOCK:
    _cursor = SHARED_CONN.cursor()
    _cursor.execute("CREATE TABLE IF NOT EXISTS flags (id TEXT, val TEXT)")
    SHARED_CONN.commit()


class Connect(threading.Thread):
    """
        Class for handling connects
    """
    def __init__(self, sock, addr):
        self.__sock = sock
        self.__addr = addr
        self.__client_ip = str(self.__addr[0])
        self.__is_kill = False
        self.__commands = {
            "put": self.__handle_command_put,
            "get": self.__handle_command_get,
            "delete": self.__handle_command_delete,
            "list": self.__handle_command_list,
            "close": self.__handle_command_close,
        }
        self.__welcome = "\n"
        self.__welcome += "Welcome to sample_service (YOUR IP: " + self.__client_ip + ")\n"
        self.__welcome += "Commands: put, get, delete, list, close\n"
        self.__welcome += "> "
        threading.Thread.__init__(self)

    def __send(self, data):
        self.__sock.send(data.encode())

    def __read(self, data=None):
        if data is not None:
            self.__sock.send(data.encode())
        resp = self.__sock.recv(1024)
        resp = resp.decode("utf-8", "ignore")
        resp = resp.strip()
        return resp

    def __check_flag_id(self, flag_id):
        if flag_id == "":
            resp = "\nFAIL: Incorrect flag_id (empty)\n"
            return False
        orig_flag_id = flag_id
        flag_id = re.search(r"\w*", flag_id).group()
        if flag_id == "" or flag_id != orig_flag_id:
            resp = "\nFAIL: Incorrect flag_id\n"
            self.__sock.send(resp.encode())
            return False
        return True

    def __flag_exists(self, cursor, flag_id):
        cursor.execute("SELECT COUNT(*) FROM flags WHERE id = ?", (flag_id,))
        total_rows = cursor.fetchone()[0]
        if total_rows == 0:
            self.__send("\nFAIL: flag_id not found\n")
            return False
        return True

    def __handle_command_list(self):
        counter = 0
        with DB_LOCK:
            cursor = SHARED_CONN.cursor()
            cursor.execute("SELECT id FROM flags", ())
            rows = cursor.fetchall()
            for row in rows:
                counter += 1
                self.__send(f"file: {row[0]}\n")
            if counter == 0:
                self.__send("*nothing*\n".encode())

    def __handle_command_close(self):
        self.__send("\nBye-bye\n\n")

    def __handle_command_put(self):
        flag_id = self.__read("flag_id = ")
        if not self.__check_flag_id(flag_id):
            return
        flag_value = self.__read("flag = ")
        if flag_value == "":
            return
        with DB_LOCK:
            cursor = SHARED_CONN.cursor()
            cursor.execute("SELECT COUNT(*) FROM flags WHERE id = ?", (flag_id,))
            total_rows = cursor.fetchone()[0]
            if total_rows == 0:
                cursor.execute("INSERT INTO flags VALUES (?,?)", (flag_id, flag_value))
            else:
                cursor.execute("UPDATE flags SET val = ? WHERE id = ?", (flag_value, flag_id))
            SHARED_CONN.commit()
        self.__send("OK\n")

    def __handle_command_get(self):
        flag_id = self.__read("flag_id = ")
        if not self.__check_flag_id(flag_id):
            return
        with DB_LOCK:
            cursor = SHARED_CONN.cursor()
            if not self.__flag_exists(cursor, flag_id):
                return
            cursor.execute("SELECT val FROM flags WHERE id = ?", (flag_id,))
            flag_value = cursor.fetchone()[0]
            self.__send("FOUND FLAG: " + flag_value + "\n")

    def __handle_command_delete(self):
        flag_id = self.__read("flag_id = ")
        if not self.__check_flag_id(flag_id):
            return
        with DB_LOCK:
            cursor = SHARED_CONN.cursor()
            if not self.__flag_exists(cursor, flag_id):
                return
            cursor.execute("DELETE FROM flags WHERE id = ?", (flag_id,))
            SHARED_CONN.commit()
            self.__send("REMOVED")

    def run(self):
        self.__sock.send(self.__welcome.encode())
        while True:
            if self.__is_kill is True:
                break
            buf = self.__read()
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
        # it's will be corrupt service
        # self.__sock.send("bye!\n".encode())
        self.__sock.close()
        CLIENT_THREADS.remove(self)

    def kill(self):
        """ kill thread client """
        if self.__is_kill is True:
            return
        self.__is_kill = True
        self.__sock.close()
        # CLIENT_THREADS.remove(self)


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
        CLIENT_THREADS.append(thr)
        thr.start()
except KeyboardInterrupt:
    print('Bye! Write me letters!')
    SERVER_SOCKET.close()
    for thr in CLIENT_THREADS:
        thr.kill()
