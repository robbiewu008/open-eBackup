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
from app.common.exter_attack import exter_attack
from app.common.clients.client_util import ProtectionServiceHttpsClient, SystemBaseHttpsClient, parse_response_data
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.copy_catalog.models.req_param import NoArchiveReq

log = logger.get_logger(__name__)


class ArchiveClient(object):

    @staticmethod
    def create_task(copy_id, policy, resource_sub_type, resource_type, sla_name=None):
        log.info(f'[ARCHIVE_TASK]:copy_id: {copy_id},resource_sub_type:{resource_sub_type}')
        archive_req = {
            "copy_id": copy_id,
            "policy": json.dumps(policy),
            "resource_sub_type": resource_sub_type,
            "resource_type": resource_type,
            "sla_name": sla_name
        }
        url = f'/v1/internal/archive-task/action/create'
        log.info(f'[ARCHIVE_TASK]:invoke api to create task, request url: {url}')
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(archive_req))
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        else:
            log.error(f'[ARCHIVE_TASK]:failed to create task')
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="failed to create task")

    @staticmethod
    def update_copy_status(copy_info):
        update_res = None
        archive_copy_req = {
            "status": copy_info["status"],
            "is_archived": True
        }
        url = f'/v1/internal/copies/{copy_info["uuid"]}/status'
        log.info(f'[ARCHIVE_TASK]:invoke api to create json, request url: {url}')
        response = ProtectionServiceHttpsClient().request("PUT", url, body=json.dumps(archive_copy_req))
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        else:
            log.info(f'[ARCHIVE_TASK]:failed to update copy info')
        return update_res

    @staticmethod
    def get_no_archive_copy_list(req_param: NoArchiveReq):
        get_res = None
        url = f'/v1/internal/copies/no-archive'
        log.info(f'[ARCHIVE_TASK]:invoke api to create json, request url: {url}')
        response = ProtectionServiceHttpsClient().request("GET", url, fields=req_param.__dict__)
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        else:
            log.error(f'[ARCHIVE_TASK]:failed to get copy info')
        return get_res

    @staticmethod
    def create_copy_archive_map(resource_id, storage_id, copy_id):
        archive_copy_req = {
            "resource_id": resource_id,
            "storage_id": storage_id,
            "copy_id": copy_id
        }
        url = f'/v1/internal/copies/action/archive'
        log.info(f'[ARCHIVE_TASK]:invoke api to create json, request url: {url}')
        response = ProtectionServiceHttpsClient().request("POST", url, body=json.dumps(archive_copy_req))
        if response.status == HTTPStatus.OK:
            log.debug(f'[ARCHIVE_TASK]:create copy archive map success')
        else:
            log.error(f'[ARCHIVE_TASK]:failed to create copy archive map')

    @staticmethod
    @exter_attack
    def query_resource(resource_id):
        url = f'/v1/internal/resource/{resource_id}'
        log.info(f'[ARCHIVE_TASK]:invoke api to query resource, request url: {url}')
        response = ProtectionServiceHttpsClient().request("GET", url)
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        else:
            log.error(f'[ARCHIVE_TASK]:failed to query resource, request url: {url}')
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="invoke api to query resource failed or timeout")

    @staticmethod
    @exter_attack
    def dispatch_archive(msg):
        url = f'/v1/internal/clusters/archive/dispatch'
        log.info(f'[ARCHIVE_TASK]:invoke api to notify archive, request url: {url}')
        response = SystemBaseHttpsClient().request("POST", url,
                                                   body=json.dumps(msg, default=lambda obj: obj.__dict__))
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        else:
            log.error(f'[ARCHIVE_TASK]:failed to query resource, '
                      f'request status: {response.status}, data: {response.data}')
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="Invoke external system api failed or timeout.")

    @staticmethod
    @exter_attack
    def query_media_set_info(media_set_id):
        url = f'/v1/internal/tape-library/media-sets/{media_set_id}'
        response = SystemBaseHttpsClient().request("GET", url)
        if response.status != HTTPStatus.OK:
            log.error(
                f"invoke api query({url}) information from error, response is response_status:{response.status}")
            return None
        return parse_response_data(response.data)

    @staticmethod
    @exter_attack
    def query_dependency_copy(copy_uuids):
        url = f'/v1/internal/archive'
        params = {
            'copyUuidList': copy_uuids
        }
        response = SystemBaseHttpsClient().request("GET", url, fields=params)
        if response.status != HTTPStatus.OK:
            log.error(
                f"invoke api query({url}) information from error, response is response_status:{response.status}")
            return None
        return parse_response_data(response.data)
