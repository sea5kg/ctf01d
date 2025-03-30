#!/usr/bin/env python
# -*- coding: utf-8 -*-
 
import socket
import threading
import re
import os

port = 4104
thrs = []

class Connect(threading.Thread):
    def __init__(self, sock, addr):
        self.sock = sock
        self.addr = addr
        self.bKill = False
        threading.Thread.__init__(self)
    def run (self):
        help_s = """
Welcome to example_py_service1
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
    
