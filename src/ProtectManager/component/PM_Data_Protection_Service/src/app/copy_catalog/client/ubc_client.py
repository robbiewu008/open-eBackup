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

from fastapi import HTTPException

from app.common.clients.client_util import is_response_status_ok, parse_response_data, UbcHttpsClient
from app.common.logger import get_logger

log = get_logger(__name__)


def delete_local_file_system_copies(resource_id):
    params = {
        "uuid": resource_id
    }
    url = '/v1/internal/dme-unified/copies/sync/delete'
    log.info(f"Delete ubc resource {resource_id} copies, url: {url}")
    response = UbcHttpsClient().request("DELETE", url, body=json.dumps(params))
    if not is_response_status_ok(response):
        log.error(f"Delete ubc resource copies error, response: {response.data}")
        raise HTTPException(response.status)
    log.info(f"Delete ubc resource copies success, response: {response.data}")
    return parse_response_data(response.data)
