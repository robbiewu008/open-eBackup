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
from app.common.clients.client_util import ProtectionServiceHttpsClient, parse_response_data, SystemBaseHttpsClient
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exter_attack import exter_attack

LOGGER = logger.get_logger(__name__)


class ResourceClient(object):

    @staticmethod
    def query_resource(resource_id):
        url = f'/v1/internal/resource/{resource_id}'
        LOGGER.info(f'invoke api to query resource, request url:{url}')
        response = ProtectionServiceHttpsClient().request("GET", url)
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(
                error=CommonErrorCodes.SYSTEM_ERROR,
                parameters=[],
                error_message="invoke request api to query resource[{resource_id}] failed or timeout"
            )
        return parse_response_data(response.data)

    @staticmethod
    def query_protected_object(resource_id: str) -> any:
        url = f'/v1/internal/protected-objects/{resource_id}'
        LOGGER.info(f'invoke api to query protected object, request url:{url}')
        response = ProtectionServiceHttpsClient().request("GET", url)
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, parameters=[],
                                       error_message="invoke protected object api failed or timeout")
        return parse_response_data(response.data)

    @staticmethod
    def update_protected_object_compliance(resource_id: str, compliance: bool):
        url = f'/v1/internal/protected-objects/compliance'
        LOGGER.info(f'Update protected object compliance, resource_id:{resource_id}, compliance:{compliance}')
        req = {
            "resource_id": resource_id,
            "compliance": compliance
        }
        response = ProtectionServiceHttpsClient().request("PUT", url, body=json.dumps(req))
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, parameters=[],
                                       error_message="invoke protected object api failed or timeout")

    @staticmethod
    def sync_protection_time(resource_id: str):
        url = f'/v1/internal/protected-objects/{resource_id}' \
              f'/action/update-last-backup-time'
        LOGGER.info(f'invoke api to sync backup time, request url:{url}')
        try:
            ProtectionServiceHttpsClient().request("PUT", url)
        except Exception:
            # 忽略调用异常，同步时间失败，不影响主流程执行
            LOGGER.exception(f'invoke api to sync backup time error')

    @staticmethod
    def sync_sla_change(sla_id: str) -> any:
        url = f'/v1/internal/protected-objects/action/sync-sla'
        LOGGER.info(f'invoke api to sync sla change, request url:{url}')
        req = {"sla_id": str(sla_id)}
        response = ProtectionServiceHttpsClient().request("PUT", url, body=json.dumps(req))
        if response.status != HTTPStatus.OK:
            LOGGER.error(f"invoke api sync sla change error, response is response_status:{response.status}")
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="invoke sync sla change failed or timeout")
        return parse_response_data(response.data)

    @staticmethod
    def sync_replica_sla_change(sla_id: str) -> any:
        url = f'/v1/internal/protected-copy-objects/action/sync-sla'
        LOGGER.info(f'invoke api to sync replica sla change, request url:{url}')
        req = {"sla_id": str(sla_id)}
        response = ProtectionServiceHttpsClient().request("PUT", url, body=json.dumps(req))
        if response.status != HTTPStatus.OK:
            LOGGER.error(f"invoke api sync replica sla change error, response is response_status:{response.status}")
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="invoke sync replica sla change failed or timeout")
        return parse_response_data(response.data)

    @staticmethod
    @exter_attack
    def query_next_backup_type_and_cause(resource_id: str) -> any:
        url = f'/v2/internal/resources/{resource_id}/next-backup-type-and-cause'
        LOGGER.info(f'invoke api to query next, request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url)
        if response.status != HTTPStatus.OK:
            LOGGER.error(f"invoke api to query next backup type and cause failed. Status: {response.status}, "
                         f"Data: {response.data}")
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       message="invoke api to query next backup type and cause failed or timeout")
        return parse_response_data(response.data)

    @staticmethod
    @exter_attack
    def clean_next_backup(resource_id: str) -> any:
        url = f'/v2/internal/resources/{resource_id}/action/clean-next-backup'
        LOGGER.info(f'invoke api to clean next backup, request url is {url}')
        response = SystemBaseHttpsClient().request("PUT", url)
        if response.status != HTTPStatus.OK:
            LOGGER.error(f"invoke api to clean next backup failed. Status: {response.status}, Data: {response.data}")
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       message="invoke api to clean next backup failed or timeout")
        LOGGER.info(f"Resource id: {resource_id}'s ext_parameters has been recovered.")

    @staticmethod
    def query_custom_resource(resource_id):
        url = f'/v2/internal/backup/getBackupResourceLock?resourceId={resource_id}'
        LOGGER.info(f'invoke api to query custom resource, request url:{url}')
        response = SystemBaseHttpsClient().request("GET", url)
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(
                error=CommonErrorCodes.SYSTEM_ERROR,
                parameters=[],
                error_message="invoke request api to query custom resource[{resource_id}] failed or timeout"
            )
        return parse_response_data(response.data)

    @staticmethod
    @exter_attack
    def is_support_data_and_log_parallel_backup(resource_id: str) -> bool:
        url = f'/v2/internal/backup/allowDataAndLogParallel?resourceId={resource_id}'
        LOGGER.info(f'invoke api to query support data and log parallel backup, request url:{url}')
        response = SystemBaseHttpsClient().request("GET", url)
        if response.status != HTTPStatus.OK:
            LOGGER.error(
                f"invoke api query support data and log parallel backup, response is response_status:{response.status}")
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="invoke query parallel backup failed or timeout")
        return parse_response_data(response.data)
