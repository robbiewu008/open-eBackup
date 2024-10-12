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
