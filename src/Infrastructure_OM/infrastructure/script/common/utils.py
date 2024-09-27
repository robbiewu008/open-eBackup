import ctypes
import re
import sys


def clear(secret):
    """
    清除secret
    :param secret: 变量
    :return: 无
    """
    if secret is not None:
        length = len(secret)
        sizeof = sys.getsizeof(secret)
        offset = sizeof - length - 1
        itemid = id(secret)
        for _ in range(3):
            ctypes.memset(itemid + offset, 0, length)


def check_path_validity(path):
    """
    校验路径合法性
    """
    regex = r'(/([a-zA-Z0-9][a-zA-Z0-9_\\-]{0,255}/)*([a-zA-Z0-9][a-zA-Z0-9_\\-]{0,255})|/)'
    if not re.match(regex, path):
        return False
    return True