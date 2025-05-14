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
import uuid

import ipaddress

from fastapi.params import Header, Path, Query

from app.common.auth.check_ath import CheckAuthModel
from app.common.enums.job_enum import JobType
from app.common.enums.rbac_enum import ResourceSetTypeEnum, OperationTypeEnum, AuthOperationEnum
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.exter_attack import exter_attack
from app.common.lock.lock import Lock
from app.common.lock.lock_manager import lock_manager
from app.common.security import jwt_utils as auth
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exception.user_error_codes import UserErrorCodes
from app.common.extension import define_page_query_api_for_model
from app.common.log.operation_log import operation_log
from app.common.logger import get_logger
from app.common.security.role_dict import RoleEnum
from app.base.db_base import database
from app.common.security.right_control import right_control
from app.common.toolkit import JobMessage
from app.resource.common.constants import EnvironmentRemoveConstants
from app.resource.discovery.res_discovery_plugin import DiscoveryManager
from app.resource.models.resource_models import EnvironmentTable
from app.resource.schemas.env_schemas import ScanEnvSchema, UpdateEnvSchema, CreateEnvSchema
from app.resource.service.common import resource_service
from app.resource.service.common.resource_service import resource_authenticate
from app.protection.object.common.constants import CommonOperationID
from app.common.concurrency import async_route, async_depend, DEFAULT_ASYNC_POOL
from app.resource.service.common.domain_resource_object_service import get_domain_id_list

discovery_router = async_route()
log = get_logger(__name__)
MANUAL_SCAN_RESOURCE_TOPIC = "job_schedule_manual_scan_resource"


def _resolve_user_info(token: str = Header(..., alias="X-Auth-Token", title="X-Auth-Token",
                                           description="访问令牌")) -> dict:
    return auth.get_user_info_from_token(token)


@exter_attack
@discovery_router.put(
    "/environments/{env_id}",
    summary="修改资源注册信息",
    response_description="The response is status code of the operation",
)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.MODIFY,
                              auth_operation_list={AuthOperationEnum.MANAGE_RESOURCE}, target="env_id")
)
@operation_log(
    name=CommonOperationID.MODIFYING_VMWARE_REGISTRATION_INFORMATION,
    target="@Resource",
    detail=('params.uuid!resource.sub_type', 'params.name', 'params.endpoint', 'params.user_name')
)
def modify_env(
        params: UpdateEnvSchema,
        env_id: str = Path(..., description="受保护环境的ID", min_length=1, max_length=64),
        user_info: dict = async_depend(_resolve_user_info)
):
    user_id = user_info.get("user-id")
    env_res = resource_service.query_resource_by_id(env_id)
    if env_res is None:
        raise EmeiStorBizException(
            error=CommonErrorCodes.OBJ_NOT_EXIST, parameters=[])
    if env_res.endpoint != params.endpoint:
        raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                   message="The environment IP can not be modified.")
    if RoleEnum.ROLE_SYS_ADMIN.value not in user_info.get("role-list") and env_res.user_id != user_id:
        raise EmeiStorBizException(
            error=UserErrorCodes.ACCESS_DENIED, parameters=[])
    dm = DiscoveryManager(ResourceSubTypeEnum(env_res.sub_type))
    params.extend_context["user_id"] = user_id
    params.uuid = env_id
    dm.modify_env(params)


define_page_query_api_for_model(
    discovery_router,
    database,
    EnvironmentTable,
    path="/environments",
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR},
    summary="查询主机资源信息列表",
    authenticate=resource_authenticate,
)


@exter_attack
@discovery_router.post(
    "/environments",
    summary="注册资源",
    response_description="The response is status code of the operation",
)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.CREATE,
                              auth_operation_list={AuthOperationEnum.MANAGE_RESOURCE})
)
@operation_log(
    name=CommonOperationID.REGISTER_VMWARE,
    target="@Resource",
    detail=('params.sub_type', 'params.name', 'params.endpoint', 'params.user_name')
)
def scan_env(
        params: CreateEnvSchema,
        user_info: dict = async_depend(_resolve_user_info)):
    pre_check_endpoint(params.endpoint)
    user_id = user_info.get("user-id")
    dm = DiscoveryManager(params.sub_type)
    scan_params = ScanEnvSchema(**params.dict())
    scan_params.job_id = str(uuid.uuid4())
    # 非管理员发现的资源，需要设置用户id
    if RoleEnum.ROLE_SYS_ADMIN.value not in user_info.get("role-list"):
        scan_params.user_id = user_id
    scan_params.domain_id = user_info.get("domain-id")
    dm.pre_check(scan_params)
    dm.create_job(scan_params, JobType.RESOURCE_SCAN.value)
    log.info(f"[Scan environment]: request_id: {scan_params.job_id}")
    DEFAULT_ASYNC_POOL.submit(dm.register_env, scan_params)


@exter_attack
@discovery_router.delete(
    "/environments/{env_id}",
    summary="删除资源",
    response_description="The response is status code of the operation",
)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.DELETE,
                              auth_operation_list={AuthOperationEnum.MANAGE_RESOURCE}, target="env_id")
)
@operation_log(
    name=CommonOperationID.REMOVE_VMWARE,
    target="@Resource",
    detail=('env_id!resource.sub_type', 'env_id!resource.name', 'env_id!resource.path')
)
def delete_env(
        env_id: str = Path(..., description="受保护环境的ID", min_length=1, max_length=64),
        user_info: dict = async_depend(_resolve_user_info)
):
    # 加分布式锁防止删除时重新扫描环境
    lock: Lock = lock_manager.get_lock(key=EnvironmentRemoveConstants.DELETE_ENV_KEY_PREFIX + env_id)
    if lock.lock(timeout=EnvironmentRemoveConstants.LOCK_TIME_OUT,
                 blocking_timeout=EnvironmentRemoveConstants.LOCK_WAIT_TIME_OUT):
        try:
            user_id = user_info.get("user-id")
            env_res = resource_service.query_resource_by_id(env_id)
            if env_res is None:
                raise EmeiStorBizException(
                    error=CommonErrorCodes.OBJ_NOT_EXIST, parameters=[])
            if RoleEnum.ROLE_SYS_ADMIN.value not in user_info.get("role-list") and env_res.user_id != user_id:
                raise EmeiStorBizException(
                    error=UserErrorCodes.ACCESS_DENIED, parameters=[])
            dm = DiscoveryManager(ResourceSubTypeEnum(env_res.sub_type))
            dm.delete_env(env_res.uuid)
        finally:
            lock.unlock()
    else:
        log.info(f"[Delete Host]: environment: {env_id} is scanning. delete failed.")
        raise EmeiStorBizException(error=ResourceErrorCodes.VSPHERE_DELETE_FAILED, parameters=[])


@exter_attack
@discovery_router.put(
    "/environments/rescan/{env_id}",
    summary="重新扫描资源环境信息",
    response_description="The response is status code of the operation",
)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.MODIFY,
                              auth_operation_list={AuthOperationEnum.MANAGE_RESOURCE}, target="env_id")
)
@operation_log(
    name=CommonOperationID.RESCAN_VMWARE_INFO,
    target="@Resource",
    detail=('env_id!resource.sub_type', 'env_id!resource.name', 'env_id!resource.path')
)
def create_rescan_env_job(
        env_id: str = Path(..., description="受保护环境的ID", min_length=1, max_length=64),
        user_info: dict = async_depend(_resolve_user_info)
):
    env_res = resource_service.query_resource_by_id(env_id)
    domain_id_list = get_domain_id_list(env_id)
    if env_res is None:
        raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST, parameters=[])
    if RoleEnum.ROLE_SYS_ADMIN.value not in user_info.get("role-list") and user_info.get(
            "domain-id") not in domain_id_list:
        raise EmeiStorBizException(
            error=UserErrorCodes.ACCESS_DENIED, parameters=[])
    dm = DiscoveryManager(ResourceSubTypeEnum(env_res.sub_type))
    if dm.check_scan_task_is_running(env_id):
        raise EmeiStorBizException(
            error=CommonErrorCodes.EXIST_SAME_TYPE_JOB_IN_RUNNING, parameters=[])
    env_params = ScanEnvSchema.parse_obj(env_res.dict())
    env_params.job_id = str(uuid.uuid4())
    env_params.domain_id = user_info.get("domain-id")
    message = JobMessage(
        topic=MANUAL_SCAN_RESOURCE_TOPIC,
        payload={
            'resId': env_id,
            'job_id': env_params.job_id
        }
    )
    dm.create_job(env_params, JobType.MANUAL_SCAN_RESOURCE.value, message)


@exter_attack
@discovery_router.put(
    "/internal/environments/rescan/{env_id}",
    summary="重新扫描资源环境信息",
    response_description="The response is status code of the operation",
)
def rescan_env(
        env_id: str = Path(..., description="受保护环境的ID", min_length=1, max_length=64),
        subtype: str = Query(..., description="环境子类型", min_length=1, max_length=64),
        job_id: str = Query(..., description="任务ID", min_length=1, max_length=64)
):
    dm = DiscoveryManager(ResourceSubTypeEnum(subtype))
    log.info(f"[Manual scan environment]: request_id: {job_id}")
    DEFAULT_ASYNC_POOL.submit(dm.manual_scan_env, env_id, job_id)


def pre_check_endpoint(endpoint: str):
    if not endpoint:
        return
    error_count = 0
    try:
        if ipaddress.IPv4Address(endpoint).is_loopback:
            raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM,
                                       message="Invalid ipV4 address, because it is loopback ip , IP: " + endpoint)
    except ValueError:
        error_count = error_count + 1

    try:
        if ipaddress.IPv6Address(endpoint).is_loopback:
            raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM,
                                       message="Invalid ipV6 address, because it is loopback ip , IP: " + endpoint)
    except ValueError:
        error_count = error_count + 1

    if error_count == 2:
        raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM,
                                   message="Invalid ip address, because it is not ipv4 and ipV6 , IP: " + endpoint)
