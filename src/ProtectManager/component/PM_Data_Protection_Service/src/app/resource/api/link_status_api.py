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
from typing import List

from fastapi import Query, Body, Header

from app.common.exter_attack import exter_attack
from app.common.logger import get_logger
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.security.trust_agent import trust_agent
from app.resource.schemas.link_status import LinkState, LinkStateUpdate
from app.resource.service.common import link_status_service as service
from app.common.concurrency import async_route

sl_api = async_route()
log = get_logger(__name__)


@exter_attack
@sl_api.get("/internal/service-links",
            summary="获取链路状态列表【内部接口】",
            status_code=200,
            response_model=List[LinkState])
@trust_agent
def query(source_role: str = Query(None, description="链路源角色", alias="sourceRole"),
          source_addr: str = Query(None, description="链路源地址", alias="sourceAddr"),
          destination_role: str = Query(None, description="链路目标角色", alias="destRole"),
          destination_addr: str = Query(None, description="链路目标地址", alias="destAddr"),
          forward_ip: str = Header(None, alias="x-forwarded-for", description="X-Forwarded-For请求头")):
    link_list = service.get_link_state(
        source_role=source_role, source_addr=source_addr,
        destination_role=destination_role, destination_addr=destination_addr)
    if not link_list:
        log.error(f'[Link Status]: info is not exist')
        raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST,
                                   parameters=[],
                                   error_message="service-links not exist")
    return link_list


@exter_attack
@sl_api.put("/internal/service-links",
            status_code=200,
            summary="修改或创建链路状态【内部接口】",
            description="ok")
@trust_agent
def modify(update_list: List[LinkStateUpdate] = Body(..., description="链路状态参数"),
           forward_ip: str = Header(None, alias="x-forwarded-for", description="X-Forwarded-For请求头")):
    for obj in update_list:
        service.modify_link_state(update_req=obj)
    return {}
