#!/usr/bin/env python3
import sys
import socket
import errno
import requests

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
port = 4103
command = sys.argv[2]
f_id = sys.argv[3]
flag = sys.argv[4]

# will be mumble (2) - for test jury
# while True: time.sleep(10);

def put_flag():
    global host, port, f_id, flag
    try:
        r = requests.post(
            'http://' + host + ':' + str(port) + '/?p=0',
            data={
                'action': 'send',
                'page': '0',
                'flagid': f_id,
                'flag': flag,
            },
            timeout=1,
            allow_redirects=True,
        )
        if r.status_code != 200:
            service_corrupt()
    except requests.exceptions.Timeout:
        service_mumble()
    except requests.exceptions.ConnectionError:
        service_down()
    except Exception as e:
        debug(e)
        service_corrupt()

def check_flag():
    global host, port, f_id, flag
    try:
        r = requests.get('http://' + host + ':' + str(port) + '/?p=0', timeout=1)
        if r.status_code != 200:
            service_corrupt()
        if f_id not in r.text or flag not in r.text:
            service_corrupt()
    except requests.exceptions.Timeout:
        service_mumble()
    except requests.exceptions.ConnectionError:
        service_down()
    except Exception as e:
        debug(e)
        service_corrupt()


if command == "put":
    put_flag()
    check_flag()
    service_up()

if command == "check":
    check_flag()
    service_up()
