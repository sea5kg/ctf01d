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
# Copyright (c) 2018-2025 Evgenii Sopov
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

 Example of service1 for ctf01d jury attack-defence system

"""

# pylint: enable=anomalous-backslash-in-string

import socket
import threading
import re
import os

HOST = ""
PORT = 4101


class Connect(threading.Thread):
    """
        handler for process connection in different thread
    """
    def __init__(self, _sock, _addr):
        self.__sock = _sock
        self.__addr = _addr
        self.__is_kill = False
        self.__dir_flags = os.path.dirname(os.path.abspath(__file__))
        self.__dir_flags += '/flags/'
        self.__dir_flags = os.path.normpath(self.__dir_flags)
        if not os.path.exists(self.__dir_flags):
            os.makedirs(self.__dir_flags)

        threading.Thread.__init__(self)

    def prepare_fullpath_to_dir_with_flag(self, flag_id):
        """
            prepare fullpath dir to flag
        """
        subdir = '__'
        if len(flag_id) > 2:
            subdir = flag_id[:2]
        ret = self.__dir_flags + "/" + subdir + "/"
        if not os.path.exists(ret):
            os.makedirs(ret)
        return ret

    def __process_command_list(self):
        """ process_command_list """
        files_len = 0
        for _dir in os.listdir(self.__dir_flags):
            for _filename in os.listdir(self.__dir_flags + "/" + _dir):
                self.__dir_flags + _filename
                resp = "file: " + _filename + "\n"
                files_len += 1
                self.__sock.send(resp.encode())
        self.__sock.send("!listended\n".encode())
        if files_len == 0:
            self.__sock.send("no files".encode())

    def __process_command_put(self):
        """ __process_command_put """
        resp = "flag_id = "
        self.__sock.send(resp.encode())
        flag_id = self.__sock.recv(1024).decode("utf-8").strip()
        if flag_id == "":
            return
        orig_flag_id = flag_id
        flag_id = re.search(r"\w*", flag_id).group()
        if flag_id == "" or flag_id != orig_flag_id:
            resp = "\nFAIL: Incorrect flag_id\n"
            print("FAIL: Ivalid flag_id " + flag_id)
            self.__sock.send(resp.encode())
            return
        resp = "flag = "
        self.__sock.send(resp.encode())
        f_text = self.__sock.recv(1024).decode("utf-8").strip()
        if f_text == "":
            return
        # create subdir in /flags/
        parent_dir = self.prepare_fullpath_to_dir_with_flag(flag_id)
        fullpath_flag_file = parent_dir + flag_id
        if os.path.isfile(fullpath_flag_file):
            print("WARN: File already exists " + fullpath_flag_file)
        with open(fullpath_flag_file, 'w') as _file:
            _file.write(f_text)
        self.__sock.send("OK".encode())

    def __process_command_get(self):
        """ __process_command_get """
        resp = "flag_id = "
        self.__sock.send(resp.encode())
        flag_id = self.__sock.recv(1024).decode("utf-8").strip()
        if flag_id == "":
            return
        orig_flag_id = flag_id
        flag_id = re.search(r"\w*", flag_id).group()
        if flag_id == "" or flag_id != orig_flag_id:
            resp = "\nFAIL: Incorrect flag_id\n"
            print("FAIL: Ivalid flag_id = " + flag_id)
            self.__sock.send(resp.encode())
            return
        parent_dir = self.prepare_fullpath_to_dir_with_flag(flag_id)
        fullpath_flag_file = parent_dir + flag_id
        if os.path.exists(fullpath_flag_file):
            with open(fullpath_flag_file) as _file:
                line = _file.readline()
            resp = "FOUND FLAG: " + line + ""
            self.__sock.send(resp.encode())
        else:
            resp = "\nFAIL: flag_id not found\n"
            self.__sock.send(resp.encode())

    def __process_command_delete(self):
        """ __process_command_delete """
        resp = "flag_id = "
        self.__sock.send(resp.encode())
        flag_id = self.__sock.recv(1024).decode("utf-8").strip()
        if flag_id == "":
            return
        orig_flag_id = flag_id
        flag_id = re.search(r"\w*", flag_id).group()
        if flag_id == "" or flag_id != orig_flag_id:
            resp = "\nFAIL: Incorrect flag_id\n"
            self.__sock.send(resp.encode())
            return
        if os.path.exists(self.__dir_flags + flag_id):
            os.remove(self.__dir_flags + flag_id)
            resp = "REMOVED"
            self.__sock.send(resp.encode())
        else:
            resp = "\nFAIL: flag_id not found\n"
            self.__sock.send(resp.encode())

    def run(self):
        help_s = "Welcome to service1!!!\n"
        help_s += "Commands: put, get, delete, list, close\n"
        help_s += "> "
        # ptrn = re.compile(r""".*(?P<name>\w*?).*""", re.VERBOSE)
        self.__sock.send(help_s.encode())
        while True:
            if self.__is_kill is True:
                break
            buf = self.__sock.recv(1024).decode("utf-8").strip()
            if buf == "":
                break
            # print(buf)
            command = re.search(r"\w*", buf).group()
            if command == "close":
                resp = "\nBye-bye\n\n"
                self.__sock.send(resp.encode())
            elif command == "list":
                self.__process_command_list()
            elif command == "put":
                self.__process_command_put()
            elif command == "get":
                self.__process_command_get()
            elif command == "delete":
                self.__process_command_delete()
            else:
                resp = "\n [" + command + "] unknown command\n\n"
                print("FAIL: unknown command [" + command + "]")
                self.__sock.send(resp.encode())
                break
        self.__is_kill = True
        self.__sock.close()
        thrs.remove(self)

    def kill(self):
        """ stop thread """
        if self.__is_kill is True:
            return
        self.__is_kill = True
        self.__sock.close()
        # thrs.remove(self)


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((HOST, PORT))
s.listen(10)

print('Start service listening ' + HOST + ':' + str(PORT))

thrs = []
try:
    while True:
        sock, addr = s.accept()
        thr = Connect(sock, addr)
        thrs.append(thr)
        thr.start()
except KeyboardInterrupt:
    print('Bye! Write me letters!')
    s.close()
    for thr in thrs:
        thr.kill()
