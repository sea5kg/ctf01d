# For Developers of Service with Vulnerabilities


## Flag Format Description

```
Flag example: c01d1fd2-133a-4713-9587-1f6a00000001
              c01d...random-data-flag.....time
              ^ prefix                    ^ timestamp is the last 8 digits
              always c01d                   (how many seconds have passed
                                            since the start of the game)
```

## Service

Serivece can be write on any language and any technoligies.

Use a best praxis: pack service like a dockerfile and docker-compose.yml

or you can also distrib only image to target culnserver:

1. Build your docker image

```
$ cd your_service_dirs
$ docker build --file "Dockerfile" --tag "somegame/your_server:0.0.1" .
```

2. Export your server as a "tar" archive (for distribution)

```
$ docker save "somegame/your_server:0.0.1" > somegame-your_server-0.0.1.tar
```

3. Import your server on vulnbox side (for a hacker's team)

```
$ docker load -i ./somegame-your_server-0.0.1.tar
```

## Checker-script

**IF YOU USING SOME DATABSE FOR A KEEPING A SESSIONS DON'T REMEBER THAT SCRIPT RUNNING IN PARALLEL PROCESSES**

Preinstalled packages for checker-script:
- python3: `requests faker grpcio grpcio-tools protobuf tzdata bs4 mimesis`
- Installed `ruby-full ruby-sqlite3`

### Prepare checker script

ctf01d will be call your checker script like `./checker.py <ip_address> <command> <flag_id> <flag>`

Where:

  * ip_address - address of a machine with this service
  * command - command, can be "put" or "check"
  * flag_id - string (10), id of the flag [a-zA-Z0-9]{10}
  * flag - uuid, value of the flag c01d[a-z0-9]{4}-[a-z0-9]{4}-[a-z0-9]{4}-[a-z0-9]{4}-[a-z0-9]{4}[0-9]{8}

Commands:

 * `put` - put the flag to the service
 * `check` - check the flag on the service

Call-examples:

 * ```./checker.py 127.0.0.1 put "1q2w3e4r5t" "c01d1fd2-133a-4713-9587-126500000010"```
 * ```./checker.py 127.0.0.1 check "1q2w3e4r5t" "c01d1fd2-133a-4713-9587-126500000010"```

Allowed return codes:

  * 101 - "service is up" (works fine)
  * 102 - "service is corrupt" (something wrong with the service functionality)
  * 104 - "service is down" (could not got response from service - also by timeout too)

Don't allowed but scripts can return:

  * none - "service is mumbled" (this state used by ctf01d for detect problems with checker script: a long time running, please fix checker-script or change `script_wait_in_sec` in ctf01d `data/config.yml`)
  * other - "checker is shit" (this state used by ctf01d for detect problems with checker script: could not run checker-script or checker-script ended with unexpected exit code)

# Checker script description

### For example checker script (in python):

```python
#!/usr/bin/env python3

import sys
import math
import socket
import random
import time
import errno
import requests

# the flag putting/checking into the service is successful
def service_up():
    print("[service is worked] - 101")
    exit(101)

# service is available (available tcp connection) but it's impossible to put/get the flag
def service_corrupt():
    print("[service is corrupt] - 102")
    exit(102)

# service is not available (maybe blocked port or service is down)
def service_down():
    print("[service is down] - 104")
    exit(104)

if len(sys.argv) != 5:
    print("\nUsage:\n\t" + sys.argv[0] + " <host> (put|check) <flag_id> <flag>\n")
    print("Example:\n\t" + sys.argv[0] + " \"127.0.0.1\" put \"abcdifghr\" \"c01d4567-e89b-12d3-a456-426600000010\" \n")
    print("\n")
    exit(0)

host = sys.argv[1]
port = 4102
command = sys.argv[2]
f_id = sys.argv[3]
flag = sys.argv[4]

# will be mumbled (2) - for test jury
# while True: time.sleep(10);

def put_flag():
    global host, port, f_id, flag
    # try put
    try:
        r = requests.post('http://' + host + ':' + str(port) + '/api/flags/' + f_id + '/' + flag)
        if r.status_code != 200:
            service_corrupt()
    except socket.timeout:
        service_down()
    except socket.error as serr:
        if serr.errno == errno.ECONNREFUSED:
            service_down()
        else:
            print(serr)
            service_corrupt()
    except Exception as e:
        print(e)
        service_corrupt()

def check_flag():
    global host, port, f_id, flag
    # try get
    flag2 = ""
    try:
        r = requests.get('http://' + host + ':' + str(port) + '/api/flags/' + f_id)
        if r.status_code != 200:
            service_corrupt()
        flag2 = r.json()['Flag']
    except socket.timeout:
        service_down()
    except socket.error as serr:
        if serr.errno == errno.ECONNREFUSED:
            service_down()
        else:
            print(serr)
            service_corrupt()
    except Exception as e:
        print(e)
        service_corrupt()

    if flag != flag2:
        service_corrupt()

if command == "put":
    put_flag()
    check_flag()
    service_up()

if command == "check":
    check_flag()
    service_up()
```

### Testing your checker with ctf01d

1. Create a file:

```
version: '3'

services:
  ctf01d_jury:
    container_name: ctf01d_jury_my_game
    image: sea5kg/ctf01d:latest
    volumes:
      - "./data_game:/usr/share/ctf01d"
    environment:
      CTF01D_WORKDIR: "/usr/share/ctf01d"
    ports:
      - "8080:8080"
    restart: always
    networks:
      - ctf01d_net

networks:
  ctf01d_net:
    driver: bridge
```

2. Call in command line:

```
$ docker compose up
```

3. Change data/config.yml

```
$ chown $USER:$USER data
$ chown $USER:$USER data/config.yml
```

Remove example services and add you checker-service, like:

```yml
checkers:
  - id: "service_ZxjQMahnoK" # work directory will be checker_service_ZxjQMahnoK
    service_name: "Service1"
    enabled: yes
    script_path: "./checker.py"
    script_wait_in_sec: 5 # max time for running script
    time_sleep_between_run_scripts_in_sec: 15 # like a round for service
```
where "service_ZxjQMahnoK" is a UNIQUE id within the game config

4. Upload your script to specified folder

Prepare folder and create ./checker.py:

```
$ mkdir data/checker_service_ZxjQMahnoK
$ chmod 775 data/checker_service_ZxjQMahnoK
$ touch data/checker_service_ZxjQMahnoK/checker.py
$ chmod +x data/checker_service_ZxjQMahnoK/checker.py
```

### If you use a different language than Python for the checker

You can write checker in any language, but you need to add requirements installation into Dockerfile with jury system

For example Dockerfile.jury with a jury:

```
FROM sea5kg/ctf01d:latest

# TODO add some packages if needs
```

Of cause you will need change in docker-compose.yml image name too.




