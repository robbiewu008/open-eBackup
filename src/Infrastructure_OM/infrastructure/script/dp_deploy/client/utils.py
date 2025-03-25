from requests import Response, codes
import logging as log
import time
import json
from functools import wraps
from enum import Enum

import consts

class CheckFailures(Enum):
    TOKEN_FAIL = "TOKEN FAIL:"
    SERVICE_FAIL = "SERVICE FAIL:"
    RESOURCE_FAIL = "RESOURCE_FAIL:"
    CONFIG_FAIL = "CONFIG_FAIL:"
    ALARMS_FAIL = "ALARMS_FAIL:"
    JOBS_FAIL = "JOBS_FAIL:"
    EXCEP = "EXCEP:"

def state_check(r: Response):
    if r.status_code != codes.ok:
        log.error(f"Status code is {r.status_code}, error msg is {r.text}")
        try:
            err_detail = json.loads(r.text).get("detail").split(":", 1)
        except Exception as e:
            msg = f"Fail to load json, {e}"
            error_dict = {"ret": False, "step": CheckFailures.EXCEP.value, "msg": {msg}}
            log.error(json.dumps(error_dict))
            return False

        err_detail_prefix = err_detail[0]
        for err in CheckFailures:
            if err_detail_prefix == err.value[:-1]:
                error_dict = {"ret": False, "step": err.value[:-1], "msg": err_detail[1:]}
                log.error(json.dumps(error_dict))
                return False
        return False
    return True



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
