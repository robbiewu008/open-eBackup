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

from app.common.clients.client_util import is_response_status_ok, UbcHttpsClient
from app.common.logger import get_logger

log = get_logger(__name__)


def remove_log_repo_whitelist_of_resource(resource_id):
    """移除资源的日志存储仓白名单"""
    params = {
        "resourceId": resource_id,
        "actions": ["removeLogRepoWhiteList"]
    }
    url = '/v1/internal/dme-unified/tasks/delete/resource'
    log.info(f"Removing log repository white list of resource(uuid=%s), url: %s, body: %s.", resource_id, url, params)
    response = UbcHttpsClient().request("POST", url, body=json.dumps(params))
    if not is_response_status_ok(response):
        log.error(f"Remove log repository white list of resource(uuid=%s) error, status: %s.", resource_id,
                  response.status)
        return
    log.info(f"Remove log repository white list of resource(uuid=%s) success.", resource_id)
