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
from typing import Dict

from fastapi import Query, status
from fastapi.params import Body

from app.common.concurrency import async_route
from app.common.exter_attack import exter_attack
from app.common.logger import get_logger
from app.resource.models.resource_models import UpgradeInfo
from app.resource.schemas.agent_penetrate_schema import AgentPenetrateUpgradeRequestSchema, \
    CheckAgentUpgradeStatusRequestSchema
from app.resource.service.common import penetrate_agent_service

agent_penetrate_router = async_route()
log = get_logger(__name__)


@exter_attack
@agent_penetrate_router.post(
    "/internal/agent/upgrade",
    status_code=status.HTTP_200_OK,
    summary="透传Agent执行客户端升级接口",
    description="执行客户端升级",
    response_model=Dict,
    responses={status.HTTP_500_INTERNAL_SERVER_ERROR: {"detail": "Internal error"}})
def agent_upgrade(upgrade_info: UpgradeInfo = Body(..., alias="agentUpdateRequest", description="创建Qos策略请求")):
    query_upgrade_req = AgentPenetrateUpgradeRequestSchema(ip=upgrade_info.ip, port=upgrade_info.port,
                                                           downloadLink=upgrade_info.download_link,
                                                           agentId=upgrade_info.agent_id,
                                                           agentName=upgrade_info.agent_name,
                                                           connect_need_proxy=upgrade_info.connect_need_proxy,
                                                           jobId=upgrade_info.jobId,
                                                           cert_secret_key=upgrade_info.certSecretKey,
                                                           new_package_size=upgrade_info.new_package_size,
                                                           packageType=upgrade_info.packageType)
    return penetrate_agent_service.action_agent_upgrade(query_upgrade_req)


@exter_attack
@agent_penetrate_router.get(
    "/internal/agent/upgrade-status",
    status_code=status.HTTP_200_OK,
    summary="透传Agent查询客户端升级状态接口",
    description="查询客户端升级状态",
    response_model=Dict,
    responses={status.HTTP_500_INTERNAL_SERVER_ERROR: {"detail": "Internal error"}})
def check_agent_upgrade_status(
        ip: str = Query(..., description="主机ip"),
        port: str = Query(..., description="主机端口"),
        connect_need_proxy: bool = Query(..., alias="canUseProxy", description="是否需要代理连接")):
    check_agent_upgrade_status_req = CheckAgentUpgradeStatusRequestSchema(ip=ip, port=port,
                                                                          connect_need_proxy=connect_need_proxy)
    return penetrate_agent_service.check_agent_upgrade_status(check_agent_upgrade_status_req)
