#!/usr/bin/env python
# _*_ coding:utf-8 _*_

from public_cbb.device_manager.storage_base_common import StorageBaseCommon
from public_cbb.device_manager.constants import StorageVersion


class StorageBlock(StorageBaseCommon):
    def __init__(self, device_info, storage_version=StorageVersion.STORAGE_V6):
        super().__init__(device_info)
        self.storage_version = storage_version
