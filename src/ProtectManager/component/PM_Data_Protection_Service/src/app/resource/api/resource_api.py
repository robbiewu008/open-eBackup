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
from http import HTTPStatus
from typing import List

from fastapi import Path, Query, Header

from app.base.db_base import database
from app.common.clients.client_util import SystemBaseHttpsClient, parse_response_data
from app.common.exter_attack import exter_attack
from app.resource.models.resource_models import ResourceTable, EnvironmentTable
from app.resource.schemas.database_models import Database
from app.resource.schemas.resource import ResourceProtectionSummary
from app.resource.service.common import resource_service
from app.resource.service.common.resource_service import resource_authenticate, resource_and_license_authenticate, \
    instance_available
from app.common import logger
from app.common.extension import define_page_query_api, define_page_query_api_for_model
from app.common.security.right_control import right_control
from app.common.security.role_dict import RoleEnum
from app.common.concurrency import async_route

LOGGER = logger.get_logger(__name__)
RESOURCE_TAG = "Resource"
resource_api = async_route()
apis = define_page_query_api(
    resource_api,
    database,
    ResourceTable,
    initiator=resource_service.resource_condition_filter,
    extra_conditions=["resource_name", "resource_set_id", "labelName", "labelList"],
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR},
    authenticate=resource_authenticate,
    excludes=EnvironmentTable,
    converter_response=resource_service.common_add_label_info_with_query_response
)
schema_types = list(api.schema for api in apis)
define_page_query_api(
    resource_api,
    database,
    ResourceTable,
    path="/internal/{path}",
    initiator=resource_service.resource_condition_filter,
    extra_conditions=["resource_name", "user_id", "labelName"],
    converter_response=resource_service.common_add_label_info_with_query_response
)

define_page_query_api_for_model(
    resource_api,
    database,
    ResourceTable,
    initiator=resource_service.global_search_condition_filter,
    extra_conditions=["labelName", "labelList"],
    path="/resource/action/search",
    tags=[RESOURCE_TAG],
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR},
    summary="全局搜索查询资源信息列表",
    authenticate=resource_and_license_authenticate,
    converter_response=resource_service.common_add_label_info_with_query_response
)

define_page_query_api_for_model(
    resource_api,
    database,
    ResourceTable,
    path="/resource",
    tags=[RESOURCE_TAG],
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR},
    summary="查询所有资源信息列表",
    authenticate=resource_authenticate
)

define_page_query_api_for_model(
    resource_api,
    database,
    ResourceTable,
    path="/internal/resource",
    tags=[RESOURCE_TAG],
    summary="查询所有资源信息列表【内部接口】",
)


# 日志记录: 用户（{0}:{1}）将资源（{2}）分配给用户（{3}）。
def _get_user_info_by_user_id(params):
    default_user_info = None
    user_id = params.get("user_id")
    url = f'/v1/internal/users/{user_id}'
    LOGGER.info(f'query user information {url}')
    response = SystemBaseHttpsClient().request("GET", url)
    if response.status != HTTPStatus.OK:
        LOGGER.info(f'failed to query the user: {user_id}')
        return default_user_info
    results = parse_response_data(response.data)
    return str(results.get("userName"))


@exter_attack
@resource_api.get(
    "/internal/resource/{resource_id}",
    status_code=200,
    tags=[RESOURCE_TAG],
    summary="根据资源ID查询资源信息【内部接口】",
)
def query_resource_by_id(resource_id: str = Path(..., description="资源ID")):
    return resource_service.query_resource_by_id(resource_id)


@resource_api.get(
    "/internal/resource/action/verify",
    status_code=200,
    tags=[RESOURCE_TAG],
    summary="根据资源ID列表和用户ID校验资源所属用户是否正确【内部接口】"
)
def verify_resource_ownership(user_id: str = Query(..., description="用户ID"),
                              resource_uuid_list: List[str] = Query(..., description="资源ID列表")):
    resource_service.verify_resource_ownership(user_id, resource_uuid_list)


@exter_attack
@resource_api.get(
    "/resource/protection/summary",
    status_code=200,
    tags=[RESOURCE_TAG],
    summary="根据资源子类型列表查询资源保护统计信息",
    response_model=ResourceProtectionSummary
)
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR})
def summary_protection_resource(sub_type: List[str] = Query(None, description="资源子类型列表"),
                                token=Header(..., alias="X-Auth-Token", title="X-Auth-Token", description="访问令牌")):
    LOGGER.debug("Getting resource protection summary.")
    return resource_service.summary_protection_resource(sub_type, token)


@exter_attack
@resource_api.put(
    "/internal/resource/action/revoke/{user_id}",
    status_code=200,
    summary="根据用户ID回收资源【内部接口】",
    tags=[RESOURCE_TAG],
)
def revoke_resource_user_id(user_id: str = Path(..., description="用户ID"), ):
    resource_service.revoke_resource_user_id(user_id)


@exter_attack
@resource_api.get(
    '/internal/agent/{host_ip}/databases',
    status_code=200,
    summary="查询代理主机注册的可用Oracle数据库信息列表【内部接口】",
    response_model=List[Database],
    response_description="The response is the list of target oracle databases",
    responses={500: {"detail": "Internal error"}},
)
def query_database_by_agent(host_ip: str = Path(..., description="代理主机IP地址", example="127.0.0.1")):
    return instance_available(host_ip)
