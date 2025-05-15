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
import time
from functools import wraps


def retry(exceptions, tries=4, wait=3, backoff=2, logger=None):
    def warp_retry(f):
        @wraps(f)
        def do_retry(*args, **kwargs):
            _tries, _wait = tries, wait
            while _tries > 1:
                try:
                    return f(*args, **kwargs)
                except exceptions as e:
                    msg = "%s, Retrying in %d seconds..." % (str(e), _wait)
                    if logger:
                        logger.warning(msg)
                    time.sleep(_wait)
                    _tries -= 1
                    _wait *= backoff
            return f(*args, **kwargs)

        return do_retry

    return warp_retry
