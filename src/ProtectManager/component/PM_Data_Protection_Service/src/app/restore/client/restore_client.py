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
from app.common.constants.constant import ServiceConstants

LOGGER = logger.get_logger(__name__)


class RestoreClient(object):

    @staticmethod
    def create_task(restore_request):
        create_res = None
        url = f'/v1/internal/restore-task/action/create'
        LOGGER.info(f'invoke api to create restore task, request url:{url}')
        data = json.loads(restore_request.json())
        response = SystemBaseHttpsClient().request("POST", url, build_exception=True, body=json.dumps(data))
        if response.status == HTTPStatus.OK:
            task = parse_response_data(response.data)
            data = task.get('data') or {}
            data['callback.abort'] = f"{ServiceConstants.PM_PROTECTION_SERVICE_URL_PREFIX}" \
                                     f"/v1/internal/restore/action/abort"
            task['data'] = data
            return task
        else:
            LOGGER.error(f'Failed to create restore task')
        return create_res

    @staticmethod
    def get_target_database(host_id, db_name):
        get_database_res = None
        url = f'/v1/internal/databases'
        LOGGER.info(f'get database info url:{url}, host_id:{host_id}')
        conditions = {"sub_type": "Oracle", "root_uuid": host_id, "name": db_name}
        conditions = json.dumps(conditions)
        params = {"page_no": 0, "page_size": 1, "conditions": conditions}
        response = ProtectionServiceHttpsClient().request("GET", url, fields=params)
        if response.status == HTTPStatus.OK:
            results = parse_response_data(response.data).get('items')
            if results:
                target_uuid = results[0].get('uuid')
                return target_uuid
        else:
            LOGGER.error(f'Failed to get database info, resource id is restoreRequest={url}')
        return get_database_res
