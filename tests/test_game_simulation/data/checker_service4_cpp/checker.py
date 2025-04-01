#!/usr/bin/python
import sys
import socket
import errno

# put-get flag to service success
def service_up():
    print("[service is worked] - 101")
    exit(101)

# service is available (available tcp connect) but protocol wrong could not put/get flag
def service_corrupt():
    print("[service is corrupt] - 102")
    exit(102)

# waited time (for example: 5 sec) but service did not have time to reply
def service_mumble():
    print("[service is mumble] - 103")
    exit(103)

# service is not available (maybe blocked port or service is down)
def service_down():
    print("[service is down] - 104")
    exit(104)

if len(sys.argv) != 5:
    print("\nUsage:\n\t" + sys.argv[0] + " <host> (put|check) <flag_id> <flag>\n")
    print("Example:\n\t" + sys.argv[0] + " \"127.0.0.1\" put \"abcdifghr\" \"123e4567-e89b-12d3-a456-426655440000\" \n")
    print("\n")
    exit(0)

def debug(err):
    pass
    # if isinstance(err, str):
    #     err = Exception(err)
    # traceback.print_exc()
    # raise err

host = sys.argv[1]
port = 4104
command = sys.argv[2]
f_id = sys.argv[3]
flag = sys.argv[4]

# will be mumble (2) - for test jury
# while True: time.sleep(10);

def put_flag():
    global host, port, f_id, flag
    # try put
    try:
        # print("try connect " + host + ":" + str(port))
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(1)
        s.connect((host, port))
        s.recv(1024)
        s.send(("put" + "\n").encode("utf-8"))
        s.recv(1024)
        s.send((f_id + "\n").encode("utf-8"))
        s.recv(1024)
        s.send((flag + "\n").encode("utf-8"))
        s.recv(1024)
        s.close()
    except socket.timeout:
        service_down()
    except socket.error as serr:
        if serr.errno == errno.ECONNREFUSED:
            service_down()
        else:
            debug(serr)
            service_corrupt()
    except Exception as e:
        debug(e)
        service_corrupt()

def check_flag():
    global host, port, f_id, flag
    # try get
    flag2 = ""
    try:
        # print("try connect " + host + ":" + str(port))
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(1)
        s.connect((host, port))
        s.recv(1024)
        s.send(("get\n").encode("utf-8"))
        s.recv(1024)
        s.send((f_id + "\n").encode("utf-8"))
        result = s.recv(1024)
        result = result.decode("utf-8", "ignore")
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
            debug(serr)
            service_corrupt()
    except Exception as e:
        debug(e)
        service_corrupt()

    if flag != flag2:
        debug('flag: [' + flag +  '] flag2: [' + str(flag2) + ']')
        service_corrupt()


if command == "put":
    put_flag()
    check_flag()
    service_up()

if command == "check":
    check_flag()
    service_up()