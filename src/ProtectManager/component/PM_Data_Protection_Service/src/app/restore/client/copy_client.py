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

LOGGER = logger.get_logger(__name__)


class CopyClient(object):

    @staticmethod
    def query_copies(page: int, size: int, conditions: dict = None, orders=None):
        query_res = None
        url = f"/v1/internal/copies"
        conditions = conditions if conditions is not None else {}
        conditions = json.dumps(conditions)

        LOGGER.info(f'invoke api to create jon, request url is {url}')
        params = {
            "page_no": page,
            "page_size": size,
            "orders": orders,
            "conditions": conditions
        }
        response = ProtectionServiceHttpsClient().request("GET", url, fields=params)
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        else:
            LOGGER.info(f'Failed to query copy info')
        return query_res

    @staticmethod
    def query_copy_info(copy_id):
        default_query_copy_res = None
        page = CopyClient.query_copies(0, 2, {"uuid": copy_id})
        if page is None:
            LOGGER.info(f'Failed to query copy info:copy_id:{copy_id}')
            return default_query_copy_res

        total = page.get("total")
        if total == 0:
            LOGGER.info(f'Not found copy:copy_id:{copy_id}')
            return default_query_copy_res
        if total > 1:
            LOGGER.info(f'Found multi copies:copy_id:{copy_id}')
            return default_query_copy_res
        return page.get("items")[0]

    @staticmethod
    def query_copy_operation_limit(copy_list, operation):
        url = f'/v1/internal/anti-ransomware/infected-copy/query-copy-operation'
        LOGGER.info(f'invoke api to query copy operation limit, request url:{url}')
        req = {
            "copies": copy_list,
            "operation": operation
        }
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(req))
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(
                error=CommonErrorCodes.SYSTEM_ERROR,
                parameters=[],
                error_message="invoke request api to query copy operation limit failed or timeout"
            )
        return parse_response_data(response.data)

    @staticmethod
    def query_copy_infected(copy_list):
        url = f'/v1/internal/anti-ransomware/infected-copy/detected-info'
        LOGGER.info(f'invoke api to query copy infected, request url:{url}')
        req = {
            "copyIds": copy_list
        }
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(req))
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(
                error=CommonErrorCodes.SYSTEM_ERROR,
                parameters=[],
                error_message="invoke request api to query copy infected failed or timeout"
            )
        return parse_response_data(response.data)
