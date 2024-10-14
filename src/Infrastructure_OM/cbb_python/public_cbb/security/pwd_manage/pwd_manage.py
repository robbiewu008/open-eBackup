# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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
