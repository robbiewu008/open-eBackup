#!/usr/bin/env python
# _*_ coding:utf-8 _*_

import ctypes
import sys


def clear(secret):
    """
    清除secret
    :param secret: 变量
    :return: 无
    """
    if secret:
        length = len(secret)
        sizeof = sys.getsizeof(secret)
        offset = sizeof - length - 1
        itemid = id(secret)
        ctypes.memset(itemid + offset, 0, length)
