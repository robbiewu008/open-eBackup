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

import urllib3
from fastapi import HTTPException

from app.common.logger import get_logger
from app.resource.client.agent_client import url_request
from app.resource.schemas.agent_penetrate_schema import AgentPenetrateUpgradeRequestSchema, \
    CheckAgentUpgradeStatusRequestSchema

log = get_logger(__name__)


def query_agent_api(method, ip, port, suffix, headers=None, body=None):
    try:
        response = url_request(method=method,
                               ip=ip,
                               port=port,
                               suffix=suffix,
                               headers=headers,
                               body=body)
    except urllib3.exceptions.HTTPError as ex:
        log.exception(f'agent_client: invoke api to query agent error')
        raise ex
    finally:
        pass
    if response.status == HTTPStatus.OK:
        value = json.loads(response.data.decode('utf-8'))
        return value
    else:
        raise HTTPException(response.status)


def action_agent_upgrade(query_upgrade_req: AgentPenetrateUpgradeRequestSchema):
    data = {
        "downloadLink": query_upgrade_req.download_link,
        "agentId": query_upgrade_req.agent_id,
        "agentName": query_upgrade_req.agent_name,
        "jobId": query_upgrade_req.jobId,
        "certSecretKey": query_upgrade_req.cert_secret_key,
        "newPackageSize": query_upgrade_req.new_package_size,
        "packageType": query_upgrade_req.packageType
    }
    body_data = json.dumps(data)
    suffix = "/agent/host/action/agent/upgrade"
    log.info(f"invoke api action agent upgrade : {suffix}")
    return query_agent_api(method="POST",
                           ip=query_upgrade_req.ip,
                           port=query_upgrade_req.port,
                           suffix=suffix,
                           body=body_data)


def check_agent_upgrade_status(check_agent_upgrade_status_req: CheckAgentUpgradeStatusRequestSchema):
    suffix = "/agent/host/action/check/status/upgrade"
    log.info(f"invoke api check agent upgrade status : {suffix}")
    return query_agent_api(method="GET",
                           ip=check_agent_upgrade_status_req.ip,
                           port=check_agent_upgrade_status_req.port,
                           suffix=suffix)
