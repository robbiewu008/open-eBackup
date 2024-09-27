from requests import Response, codes
import logging as log
import consts
import time
from functools import wraps


def check_result(r: Response, msg: str):
    if r.status_code != codes.ok:
        log.error(f"{msg}, {r.text}")
        r.raise_for_status()


def make_dpserver_address(ip: str) -> str:
    if ip.endswith(consts.DATAPROTECT_SERVER_POST):
        return ip
    return ip+f":{consts.DATAPROTECT_SERVER_POST}"


def make_pacifc_address(ip: str) -> str:
    if ip.endswith(consts.PACIFIC_PORT):
        return ip
    return ip+f':{consts.PACIFIC_PORT}'


def retry(exception_to_check=BaseException, tries=3, delay=5):
    """ 重试装饰器，用于在特定异常下进行函数级重试 """
    def deco_retry(f):
        @wraps(f)
        def f_retry(*args, **kwargs):
            n = 1
            while n <= tries:
                try:
                    return f(*args, **kwargs)
                except exception_to_check:
                    time.sleep(delay)
                    n += 1
            return f(*args, **kwargs)

        return f_retry

    return deco_retry
