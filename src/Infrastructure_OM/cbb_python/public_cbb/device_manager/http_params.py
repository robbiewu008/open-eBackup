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

class SessionInfo:
    def __init__(
        self,
        base_url: str = '',
        device_id: str = '',
        token: str = '',
        cookie: str = '',
    ):
        self.base_url = base_url
        self.device_id = device_id
        self.token = token
        self.cookie = cookie
