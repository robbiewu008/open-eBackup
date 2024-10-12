#!/usr/bin/env python
# _*_ coding:utf-8 _*_

import time
from functools import wraps

from public_cbb.log.logger import get_logger
from public_cbb.security.anonym_utils.anonymity import Anonymity

logger = get_logger()


def retry(exception_to_check=BaseException, tries=3, delay=5):
    """ 重试装饰器，用于在特定异常下进行函数级重试 """
    def deco_retry(f):
        @wraps(f)
        def f_retry(*args, **kwargs):
            n = 1
            while n <= tries:
                try:
                    return f(*args, **kwargs)
                except exception_to_check as e:
                    logger.warn(
                        f'Caught exception: \'{Anonymity.process(str(e))}\','
                        f'then after {delay} seconds will do the {n}th retry')
                    time.sleep(delay)
                    n += 1
            return f(*args, **kwargs)

        return f_retry

    return deco_retry


def exter_attack(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        return func(*args, **kwargs)
    return wrapper
