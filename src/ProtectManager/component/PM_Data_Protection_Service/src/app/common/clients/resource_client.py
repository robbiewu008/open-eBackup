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

from app.common import logger
from app.common.clients.client_util import (
    ProtectionServiceHttpsClient, SystemBaseHttpsClient, is_response_status_ok, parse_response_data
)
from app.common.enums.resource_enum import ResourceSubTypeWithOrderEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exter_attack import exter_attack
from app.protection.object.models.projected_object import ProtectedObject

LOGGER = logger.get_logger(__name__)


class ResourceClient(object):

    @staticmethod
    def query_resource(resource_id):
        url = f'/v1/internal/resource/{resource_id}'
        LOGGER.info(f'invoke api to query resource, request url is {url}')
        response = ProtectionServiceHttpsClient().request("GET", url)
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="invoke sla api failed or timeout")
        return parse_response_data(response.data)

    @staticmethod
    @exter_attack
    def query_v2_resource(resource_id):
        url = f'/v2/internal/resources/{resource_id}'
        LOGGER.info(f'invoke api to query resource, request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url)
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        else:
            LOGGER.error(f'Failed to query resource info, resource id is {resource_id}')

    @staticmethod
    @exter_attack
    def query_v2_resource_list(conditions: dict = None):
        results = []
        page_no = 0
        page_size = 200
        url = f'/v2/internal/resources'
        conditions_not_emyty = conditions if conditions is not None else {}
        conditions_json = json.dumps(conditions_not_emyty)
        LOGGER.info(f'invoke api to query resources, request url is {url}')
        while page_no >= 0:
            params = {"page_no": page_no, "page_size": page_size, "conditions": conditions_json}
            response = SystemBaseHttpsClient().request("GET", url, fields=params)
            if not is_response_status_ok(response):
                raise EmeiStorBizException(
                    CommonErrorCodes.SYSTEM_ERROR,
                    message=f"Failed to get request , url is {url}")
            data = parse_response_data(response.data)
            items: list = data.get("records", [])
            results.extend(items)
            if len(items) < page_size:
                page_no = -1
            else:
                page_no += 1
        return results


    @staticmethod
    def query_resource_list(conditions: dict = None):
        results = []
        page_no = 0
        page_size = 200
        url = f'/v1/internal/resource'
        conditions = conditions if conditions is not None else {}
        conditions = json.dumps(conditions)
        LOGGER.info(f'invoke api to query resources, request url is {url}')
        while page_no >= 0:
            params = {"page_no": page_no, "page_size": page_size, "conditions": conditions}
            response = ProtectionServiceHttpsClient().request("GET", url, fields=params)
            if not is_response_status_ok(response):
                raise EmeiStorBizException(
                    CommonErrorCodes.SYSTEM_ERROR,
                    message=f"Failed to get request , url is {url}")
            data = parse_response_data(response.data)
            items: list = data.get("items", [])
            results.extend(items)
            if len(items) < page_size:
                page_no = -1
            else:
                page_no += 1
        return results

    @staticmethod
    def query_vm_disk(resource_id):
        url = f'/v1/internal/virtual-machines/{resource_id}/disks'
        LOGGER.info(f'invoke api to query vm disk info, request url is {url}')
        response = ProtectionServiceHttpsClient().request("GET", url)
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        else:
            LOGGER.error(f'Failed to query vm disk info, resource_id is {resource_id}, url is {url}')

    @staticmethod
    @exter_attack
    def query_database_target_host(copy_id, system_change_number, time, resource_id):
        url = f'/v1/eb/internal/copies/extended_parameter'
        target_req = {
            "id": copy_id,
            "scn": system_change_number,
            "time": time,
            "databaseId": resource_id
        }
        LOGGER.info(f'invoke api to query target host information, request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url, fields=target_req)
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        else:
            LOGGER.error(f'Failed to query oracle target databases info')
            raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST, parameters=[])

    @staticmethod
    @exter_attack
    def umount_agent_and_lan_free(projected_object: ProtectedObject):
        sub_type = projected_object.sub_type
        if sub_type not in [ResourceSubTypeWithOrderEnum.INFORMIX_SINGLE_INSTANCE.value[0],
                            ResourceSubTypeWithOrderEnum.INFORMIX_CLUSTER_INSTANCE.value[0]]:
            return parse_response_data("")

        resource_id = projected_object.resource_id
        url = f'/v1/internal/host-agent/action/umount?resource_id={resource_id}'
        LOGGER.info(f'Invoke api to umount agent and lan free, request url is %s', url)
        response = SystemBaseHttpsClient().request("PUT", url)
        if response.status != HTTPStatus.OK:
            LOGGER.error(f'Failed umount agent and lan free, resource id is %s', resource_id)
        return parse_response_data(response.data)
