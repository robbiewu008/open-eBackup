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
from http import HTTPStatus
from typing import List

from app.common import logger
from app.common.clients.client_util import SystemBaseHttpsClient
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException

LOGGER = logger.get_logger(__name__)


class RBACClient(object):
    @staticmethod
    # 目前只有resource资源集类型的资源需要指定subType 其他类型不指定 默认空字符串
    # 后续需要指定type时请同步修改java ResourceSetRelationExecuteProvider#buildDefaultResourceSetResourceBo 方法
    def add_resource_set_relation(resource_set_relation_info):
        url = f'/v1/internal/resource-set/add-relation'
        LOGGER.info(
            f'add resource set relation, resource_object_id:{resource_set_relation_info.resource_object_id}, '
            f'domain_id_list:{resource_set_relation_info.domain_id_list}, '
            f'sub_type:{resource_set_relation_info.sub_type}, parent_uuid:{resource_set_relation_info.parent_uuid}')
        req = {
            "resource_object_id": resource_set_relation_info.resource_object_id,
            "resource_set_type": resource_set_relation_info.resource_set_type,
            "scope_module": resource_set_relation_info.scope_module,
            "domain_id_list": resource_set_relation_info.domain_id_list,
            "parent_uuid": resource_set_relation_info.parent_uuid,
            "resource_sub_type": resource_set_relation_info.sub_type
        }
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(req))
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, parameters=[],
                                       error_message="invoke add resource set relation api failed or timeout")

    @staticmethod
    def delete_resource_set_relation(resource_object_id_list: List[str], resource_set_type: str):
        url = f'/v1/internal/resource-set/delete-relation'
        LOGGER.info(f'delete resource set relation, resource_object_id size:{len(resource_object_id_list)}')
        req = {
            "resource_object_id_list": resource_object_id_list,
            "resource_set_type": resource_set_type
        }
        response = SystemBaseHttpsClient().request("DELETE", url, body=json.dumps(req))
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, parameters=[],
                                       error_message="invoke delete resource set relation api failed or timeout")
