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
import re
import time
from http import HTTPStatus

from app.common import logger
from app.common.clients.client_util import SystemBaseHttpsClient, parse_response_data
from app.common.enums.sla_enum import BackupTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exter_attack import exter_attack

LOG_REPO = 3

LOGGER = logger.get_logger(__name__)


class AntiRansomwareClient(object):

    @staticmethod
    @exter_attack
    def query_policy_by_resource_id(resource_id: str) -> any:
        url = f'/v1/internal/anti-ransomware/{resource_id}/policy'
        LOGGER.info(f'invoke api to query anti ransomware policy, resource id is {resource_id}, request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url)
        if response.status != HTTPStatus.OK:
            LOGGER.error(f"invoke api to query sla failed. Status: {response.status}, Data: {response.data}")
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       message="invoke sla api failed or timeout")
        return parse_response_data(response.data)

    @staticmethod
    def create_copy_detection(copy_id: str) -> any:
        url = f'/v1/internal/anti-ransomware/action/detect'
        LOGGER.info(f'invoke api to create copy detection, request url:{url}')
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps({"copyId": copy_id}))
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="invoke sla api failed or timeout")
        return parse_response_data(response.data)

    @staticmethod
    def get_copy_expire_status(cur_copy) -> any:
        url = f'/v1/internal/anti-ransomware/worm/expire'
        expire_timestamp = int(time.mktime(cur_copy.worm_expiration_time.timetuple()))
        copy_properties = json.loads(cur_copy.properties)
        if cur_copy.backup_type == BackupTypeEnum.log.value:
            repositories = copy_properties.get("repositories", [])
            parent_name = AntiRansomwareClient.fill_file_system_name(cur_copy, repositories)
        else:
            parent_name = copy_properties.get("snapshots")[0].get("parentName")
        copy_expire_req = {
            "fileSystemName": parent_name,
            "expireTimeStamp": expire_timestamp
        }
        copy_expire_req.update({"copyId": cur_copy.uuid})
        LOGGER.info(f'invoke api to get copy expire status, request url:{url}, request body:{copy_expire_req}')
        response = SystemBaseHttpsClient().request("GET", url, body=json.dumps(copy_expire_req))
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="invoke sla api failed or timeout")
        return parse_response_data(response.data).get("isExpired")

    @staticmethod
    def fill_file_system_name(cur_copy, repositories):
        parent_name = ""
        for repo in repositories:
            remote_paths = repo.get("remotePath", [])
            if repo.get("type") == LOG_REPO and remote_paths:
                # /Database_7e1e0bb6434e4ff2b87f4127639f8605_LogRepository_su0/
                # 1af25f83-7476-446e-9dac-27d324c321ba
                path = remote_paths[0].get("path", "")
                LOGGER.info(f"Get copy: {cur_copy.uuid} expire status, path: {path}")

                # 使用正则提取父级名称
                match = re.search(r'/([^/]+)/', path)
                if match:
                    parent_name = match.group(1)
                break  # 找到有效路径后退出循环
        return parent_name
