#
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
#

import json
import requests

from tdsql.logger import log


def request_post(url, body, headers):
    result = requests.post(url, json=body, headers=headers, verify=False)
    try:
        ret_body = json.loads(result.content.decode())
    except Exception as err:
        log.err(f"Err at POST: {err}")
        return False, {}, {}
    ret_headers = dict(result.headers)
    return True, ret_body, ret_headers
