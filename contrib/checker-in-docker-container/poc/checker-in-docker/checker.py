import socket
import errno

from flask import Flask, request, jsonify


PORT = 4101


# put-get flag to service success
def service_up():
    print("[service is worked] - 101")
    return jsonify(101)

# service is available (available tcp connect) but protocol wrong could not put/get flag
def service_corrupt():
    print("[service is corrupt] - 102")
    return jsonify(102)

# waited time (for example: 5 sec) but service did not have time to reply
def service_mumble():
    print("[service is mumble] - 103")
    return jsonify(103)

# service is not available (maybe blocked port or service is down)
def service_down():
    print("[service is down] - 104")
    return jsonify(104)


app = Flask(__name__)


@app.route('/put', methods=['POST'])
def put():
    host, f_id, flag = request.json
    try:
        # print("try connect " + host + ":" + str(port))
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(1)
        s.connect((host, PORT))
        _ = s.recv(1024).decode("utf-8")
        # print(result)
        s.send("put\n".encode())
        _ = s.recv(1024).decode("utf-8")
        s.send(str(f_id + "\n").encode())
        _ = s.recv(1024).decode("utf-8")
        s.send(str(flag + "\n").encode())
        _ = s.recv(1024).decode("utf-8")
        s.close()
    except socket.timeout:
        return service_mumble()
    except socket.error as serr:
        if serr.errno == errno.ECONNREFUSED:
            return service_down()
        else:
            print(serr)
            return service_corrupt()
    except Exception as e:
        print(e)
        return service_corrupt()
    return service_up()


@app.route('/check', methods=['POST'])
def check():
    host, f_id, flag = request.json
    try:
        # print("try connect " + host + ":" + str(port))
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(10)
        s.connect((host, PORT))
        result = s.recv(1024).decode("utf-8")
        # print(result)
        s.send("get\n".encode())
        result = s.recv(1024).decode("utf-8")
        s.send(str(f_id + "\n").encode())
        result = s.recv(1024).decode("utf-8")
        flag2 = result.strip()
        flag2 = flag2.split("FOUND FLAG: ")
        if len(flag2) == 2:
            flag2 = flag2[1]
        else:
            flag2 = ''
        s.close()
    except socket.timeout:
        return service_down()
    except socket.error as serr:
        if serr.errno == errno.ECONNREFUSED:
            return service_down()
        else:
            print(serr)
            return service_corrupt()
    except Exception as e:
        print(e)
        return service_corrupt()

    if flag != flag2:
        return service_corrupt()

    return service_up()
