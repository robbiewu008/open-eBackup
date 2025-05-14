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
import uuid

from app.resource.service.vmware.storage_service import StorageService
from app.common.logger import get_logger

logger = get_logger(__name__)

_storage_cache_map = {}


def get_storage_service(storage_ip: str, username: str, password: str, port=8088):
    instance_uuid = str(uuid.uuid5(uuid.NAMESPACE_X500, storage_ip + str(port) + username))
    storage_service = _storage_cache_map.get(instance_uuid, {})
    if storage_service and storage_service.is_storage_service_valid():
        return storage_service
    else:
        return build_storage_service(storage_ip, username, password, port)


def build_storage_service(storage_ip: str, username: str, password: str, port=8088):
    instance_uuid = str(uuid.uuid5(uuid.NAMESPACE_X500, storage_ip + str(port) + username))
    storage_service = StorageService(storage_ip, username, password, port)
    _storage_cache_map[instance_uuid] = storage_service
    return storage_service

