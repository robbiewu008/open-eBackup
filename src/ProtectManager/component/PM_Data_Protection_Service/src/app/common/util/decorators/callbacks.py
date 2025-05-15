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
from functools import wraps


def error_callback(callback, logger=None):
    def warp_callback(func):
        @wraps(func)
        def do_with_callback(*args, **kwargs):
            try:
                return func(*args, **kwargs)
            except Exception as es:
                if logger:
                    logger.exception(f"execute {func.__name__} error.")
                kwargs['exception'] = es
                callback(*args, **kwargs)

        return do_with_callback

    return warp_callback
