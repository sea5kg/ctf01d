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

import socket
import threading
import re
import os

port = 4103
thrs = []

class Connect(threading.Thread):
    def __init__(self, sock, addr):
        self.sock = sock
        self.addr = addr
        self.bKill = False
        threading.Thread.__init__(self)
    def run (self):
        help_s = """
Welcome to service3_py
Commands: put, get, delete, list, close
> """
        self.sock.send(help_s.encode())
        while True:
            if self.bKill == True:
                break
            buf = self.sock.recv(1024)
            buf = buf.decode("utf-8", "ignore")
            buf = buf.strip()
            if buf == "":
                break
            command = re.search( r"\w*", buf).group()
            if command == "close":
                resp = "\nBye-bye\n\n"
                self.sock.send(resp.encode())
                break
            elif command == "list":
                for f in os.listdir('flags/'):
                    resp = "file: " + f + "\n"
                    self.sock.send(resp.encode())
                break
            elif command == "put":
                resp = "flag_id = "
                self.sock.send(resp.encode())
                f_id = self.sock.recv(1024)
                f_id = f_id.decode("utf-8", "ignore")
                f_id = f_id.strip()
                if f_id == "":
                    break
                orig_flag_id = f_id
                f_id = re.search( r"\w*", f_id).group()
                if f_id == "" or f_id != orig_flag_id:
                    resp = "\nFAIL: Incorrect flag_id\n"
                    self.sock.send(resp.encode())
                    break
                resp = "flag = "
                self.sock.send(resp.encode())
                f_text = self.sock.recv(1024)
                f_text = f_text.decode("utf-8", "ignore")
                if f_text == "":
                    break
                f = open('flags/'+f_id, 'w')
                f.write(f_text)
                self.sock.send("OK".encode())
                f.close()
                break
            elif command == "get":
                resp = "flag_id = "
                self.sock.send(resp.encode())
                f_id = self.sock.recv(1024)
                f_id = f_id.decode("utf-8", "ignore")
                f_id = f_id.strip()
                if f_id == "":
                    break
                orig_flag_id = f_id
                f_id = re.search( r"\w*", f_id).group()
                if f_id == "" or f_id != orig_flag_id:
                    resp = "\nFAIL: Incorrect flag_id\n"
                    self.sock.send(resp.encode())
                    break
                if os.path.exists('flags/' + f_id):
                    f = open('flags/' + f_id, 'r')
                    line = f.readline()
                    f.close()
                    resp = "FOUND FLAG: " + line + ""
                    self.sock.send(resp.encode())
                else:
                    resp = "\nFAIL: flag_id not found\n"
                    self.sock.send(resp.encode())
                break
            elif command == "delete":
                resp = "flag_id = "
                self.sock.send(resp.encode())
                f_id = self.sock.recv(1024)
                f_id = f_id.decode("utf-8", "ignore")
                f_id = f_id.strip()
                if f_id == "":
                    break
                orig_flag_id = f_id
                f_id = re.search( r"\w*", f_id).group()
                if f_id == "" or f_id != orig_flag_id:
                    resp = "\nFAIL: Incorrect flag_id\n"
                    self.sock.send(resp.encode())
                    break
                if os.path.exists('flags/' + f_id):
                    os.remove('flags/' + f_id) 
                    resp = "REMOVED"
                    self.sock.send(resp.encode())
                else:
                    resp = "\nFAIL: flag_id not found\n"
                    self.sock.send(resp.encode())
                break
            elif command:
                resp = "\n ["+ command + "] unknown command\n\n"
                self.sock.send(resp.encode())
                break
        self.bKill = True
        self.sock.close()
        thrs.remove(self)

    def kill(self):
        if self.bKill == True:
            return
        self.bKill = True
        self.sock.close()
        # thrs.remove(self)
 
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind(('0.0.0.0', port))
s.listen(10)

if not os.path.exists("flags"):
    os.makedirs("flags")
        
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
    
