import json
import logging
import socket
from concurrent.futures import ThreadPoolExecutor
from concurrent.futures import as_completed
from urllib.error import URLError
from urllib.request import Request
from urllib.request import urlopen

from configs import MAX_WORKERS
from constants import CHECKER_SERVICE_DOWN
from constants import CHECKER_SERVICE_ERROR

logger = logging.getLogger(__name__)


def build_service_host(mask: str, host_id: str) -> str:
    return mask.format(host_id)


def _call_checker(
    checker_host: str,
    checker_port: int,
    command: str,
    service_host: str,
    flag_id: str,
    flag_value: str,
    timeout: float,
) -> int | str:
    url = f'http://{checker_host}:{checker_port}/{command}'
    payload = json.dumps([service_host, flag_id, flag_value]).encode('utf-8')
    request = Request(
        url,
        data=payload,
        headers={'Content-Type': 'application/json'},
        method='POST',
    )
    try:
        with urlopen(request, timeout=timeout) as response:
            return json.loads(response.read().decode('utf-8'))
    except (TimeoutError, socket.timeout):
        logger.warning('Checker request timed out: %s', url)
        return CHECKER_SERVICE_DOWN
    except URLError as e:
        if isinstance(e.reason, (TimeoutError, socket.timeout)):
            logger.warning('Checker request timed out: %s', url)
            return CHECKER_SERVICE_DOWN
        raise


def _process_flag(
    host_id: str,
    flag_id: str,
    flag_value: str,
    mask: str,
    command: str,
    checker_host: str,
    checker_port: int,
    timeout: float,
) -> tuple[str, int | str]:
    try:
        service_host = build_service_host(mask, host_id)
        status = _call_checker(
            checker_host,
            checker_port,
            command,
            service_host,
            flag_id,
            flag_value,
            timeout,
        )

    except Exception:
        logger.exception('Failed to process flag for host_id=%r', host_id)
        return host_id, CHECKER_SERVICE_ERROR

    logger.debug(
        'Flag for host_id=%r (service=%r): status=%r',
        host_id,
        service_host,
        status,
    )
    return host_id, status

def forward_flags(
    timeout: float,
    mask: str,
    command: str,
    checker_host: str,
    checker_port: int,
    flags: list,
) -> dict[str, int | str]:
    if not flags:
        return {}

    max_workers = min(len(flags), MAX_WORKERS)
    results: dict[str, int | str] = {}

    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = [
            executor.submit(
                _process_flag,
                host_id,
                flag_id,
                flag_value,
                mask,
                command,
                checker_host,
                checker_port,
                timeout,
            )
            for host_id, flag_id, flag_value in flags
        ]
        for future in as_completed(futures):
            host_id, status = future.result()
            results[host_id] = status

    return results
