import json
import logging
import socketserver

from configs import DEFAULT_TIMEOUT
from configs import HOST
from configs import PORT
from constants import TIMEOUT_KEY
from constants import MASK_KEY
from constants import COMMAND_KEY
from constants import CHECKER_KEY
from constants import FLAGS_KEY
from flag_forwarder import forward_flags

logging.basicConfig(
    format="%(asctime)s [name] <thr-%(thread)d> %(levelname)s: %(message)s", level=logging.DEBUG
)
logger = logging.getLogger(__name__)


def forward(data: dict) -> dict:
    timeout = data.get(TIMEOUT_KEY, DEFAULT_TIMEOUT)
    mask = data.get(MASK_KEY, '')
    command = data.get(COMMAND_KEY)
    flags = data.get(FLAGS_KEY, [])
    checker_host, checker_port = data.get(CHECKER_KEY)

    if command not in ('put', 'check'):
        raise NotImplementedError(f'Command {command} is not implemented')

    logger.info(
        'Got such data: timeout=%r, mask=%r, command=%r, len(flags)=%d, checker_host=%r, checker_port=%d',
        timeout,
        mask,
        command,
        len(flags),
        checker_host,
        checker_port,
    )
    return forward_flags(
        timeout=timeout,
        mask=mask,
        command=command,
        checker_host=checker_host,
        checker_port=checker_port,
        flags=flags,
    )


class ThreadedTCPRequestHandler(socketserver.BaseRequestHandler):
    def handle(self):
        try:
            data = []
            while True:
                chunk = self.request.recv(1024)
                if not chunk.strip():
                    break
                data.append(chunk)
            data = b''.join(data)
            data = json.loads(data)
            result = forward(data)
            self.request.sendall(json.dumps(result, sort_keys=True).encode('utf-8'))
        except Exception as exc:
            logger.exception('Unhandled exception')
            error_data = {'error': repr(exc)}
            self.request.sendall(json.dumps(error_data).encode('utf-8'))


class ThreadedTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    allow_reuse_address = True


if __name__ == '__main__':
    server = ThreadedTCPServer((HOST, PORT), ThreadedTCPRequestHandler)
    with server:
        logger.info('Socket server for forwarding flags is started (%r, %d)', HOST, PORT)
        server.serve_forever()
