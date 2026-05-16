import errno
import logging
import socket

from flask import Flask, request, jsonify


PORT = 4101

logging.basicConfig(
    format="%(asctime)s [name] <thr-%(thread)d> %(levelname)s: %(message)s", level=logging.DEBUG
)
logger = logging.getLogger(__name__)


# put-get flag to service success
def service_up():
    logger.info("[service is worked] - 101")
    return jsonify(101)

# service is available (available tcp connect) but protocol wrong could not put/get flag
def service_corrupt():
    logger.info("[service is corrupt] - 102")
    return jsonify(102)

# waited time (for example: 5 sec) but service did not have time to reply
def service_mumble():
    logger.info("[service is mumble] - 103")
    return jsonify(103)

# service is not available (maybe blocked port or service is down)
def service_down():
    logger.info("[service is down] - 104")
    return jsonify(104)


app = Flask(__name__)


@app.before_request
def log_request():
    if request.method != 'POST' or request.path not in ('/put', '/check'):
        return
    logger.info('Request %s %s: data=%r', request.method, request.path, request.json)


@app.after_request
def log_response(response):
    if request.method != 'POST' or request.path not in ('/put', '/check'):
        return response
    logger.info(
        'Response %s %s: status=%d body=%s',
        request.method,
        request.path,
        response.status_code,
        response.get_data(as_text=True),
    )
    return response


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
            logger.exception('Socket error on put: %s', serr)
            return service_corrupt()
    except Exception as e:
        logger.exception('Unhandled exception on put: %s', e)
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
            logger.exception('Socket error on check: %s', serr)
            return service_corrupt()
    except Exception as e:
        logger.exception('Unhandled exception on check: %s', e)
        return service_corrupt()

    if flag != flag2:
        return service_corrupt()

    return service_up()
