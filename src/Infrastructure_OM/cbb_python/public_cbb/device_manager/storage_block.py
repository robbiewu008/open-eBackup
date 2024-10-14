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

from public_cbb.device_manager.storage_base_common import StorageBaseCommon
from public_cbb.device_manager.constants import StorageVersion


class StorageBlock(StorageBaseCommon):
    def __init__(self, device_info, storage_version=StorageVersion.STORAGE_V6):
        super().__init__(device_info)
        self.storage_version = storage_version
