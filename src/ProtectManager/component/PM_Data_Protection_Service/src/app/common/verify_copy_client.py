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
from typing import TypeVar


from app.common import logger
from app.common.clients.client_util import SystemBaseHttpsClient, is_response_status_ok
from app.common.http import LONG_RETRY_POLICY

log = logger.get_logger(__name__)
T = TypeVar('T')


def send_verify_copy(copy_id: str, user_id: str, agents: str):
    log.info(f"send_verify_copy:{copy_id} user_id:{user_id} agents:{agents}")
    verify_copy_url = f"/v2/internal/copies/{copy_id}/action/verify"
    request_body = {"agents": agents, "userId": user_id}
    response = SystemBaseHttpsClient(retries=LONG_RETRY_POLICY).request(
        "POST", verify_copy_url, body=json.dumps(request_body))
    if is_response_status_ok(response):
        log.info(f"{copy_id} send_verify_copy success.")
    else:
        log.error(f"response.data:{response.data}")
