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

from public_cbb.log.logger import get_logger
from public_cbb.device_manager.device_info import DeviceInfo
from public_cbb.device_manager.storage_nas import StorageNas
from public_cbb.device_manager.storage_block import StorageBlock
from public_cbb.device_manager.constants import StorageProtocol, StorageType, NasShareType, StorageVersion

log = get_logger()


class DeviceManager:
    @staticmethod
    def create_device_inst(info: DeviceInfo, storage_type: StorageType, protocol: StorageProtocol,
                           storage_version=StorageVersion.STORAGE_V6, nas_share_type=NasShareType.NFS):
        if storage_type == StorageType.LOCAL:
            if protocol == StorageProtocol.SAN:
                device = StorageBlock(info, storage_version)
                return device if device.init_success else None
            elif protocol == StorageProtocol.NAS:
                device = StorageNas(info, nas_share_type)
                return device if device.init_success else None
            else:
                log.error("Protocol's type is not supported!")
                return None
        log.error("Storage's type is not supported!")
        return None
