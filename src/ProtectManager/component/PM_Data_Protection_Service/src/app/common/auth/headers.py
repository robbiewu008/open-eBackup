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
from .user_info import user_info_from_token


def header(auth_token: str, req_id: str, additional=None):
    if additional is None:
        additional = {}
    ret = {}
    ret.update(additional)
    ret.update(
        {'X-Auth-Token': auth_token, 'X-Request-ID': req_id, }
    )
    ret.update(user_info_from_token(auth_token))
    return ret
