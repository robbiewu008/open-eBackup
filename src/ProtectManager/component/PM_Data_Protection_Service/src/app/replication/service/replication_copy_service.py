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
import json

from app.common import logger
from app.common.enums.copy_enum import CopyFormatEnum
from app.copy_catalog.service.curd.copy_query_service import query_copy_by_resource_id, query_last_copy_by_resource_id
from app.backup.client.resource_client import ResourceClient



log = logger.get_logger(__name__)


def get_resource_obj_from_copy(resource_id: str):
    copy = query_copy_by_resource_id(resource_id=resource_id, generated_by=None)
    return json.loads(copy.as_dict().get("resource_properties"))


def get_last_resource_obj_from_copy(resource_id: str):
    copy = query_last_copy_by_resource_id(resource_id=resource_id, generated_by=None)
    return json.loads(copy.as_dict().get("resource_properties"))


def get_resource_obj(resource_obj, resource_id):
    query_resource = ResourceClient.query_resource(resource_id=resource_id)
    if not resource_obj:
        return query_resource if query_resource else get_resource_obj_from_copy(resource_id)
    else:
        resource_info = query_resource if query_resource else get_last_resource_obj_from_copy(resource_id)
        resource_obj["name"] = resource_info.get("name")
        return resource_obj


def is_reverse_replication(copy_id):
    return bool(copy_id)


def is_directory_copy(copy):
    if not copy:
        return False
    copy_format = json.loads(copy.properties).get("format")
    log.debug(f"Copy: {copy.uuid} format: {copy_format}")
    return copy_format is not None and copy_format == CopyFormatEnum.INNER_DIRECTORY.value
