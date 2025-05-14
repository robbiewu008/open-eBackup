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
from typing import List, Optional

from fastapi import Query, Body, Path, Header
from sqlalchemy.orm import Session

from app.base.resource_consts import LinuxOsTypeEnum
from app.common.auth.check_ath import CheckAuthModel
from app.common.clients.client_util import ProtectionServiceHttpsClient, parse_response_data
from app.common.concurrency import async_route, async_depend
from app.common.enums.rbac_enum import AuthOperationEnum, ResourceSetTypeEnum, OperationTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exter_attack import exter_attack
from app.common.log.operation_log import operation_log
from app.common.logger import get_logger
from app.common.schemas.common_schemas import BasePage
from app.common.security.jwt_utils import get_user_info_from_token
from app.common.security.right_control import right_control
from app.common.security.role_dict import RoleEnum
from app.common.util.cleaner import clear
from app.protection.object.common import db_config
from app.protection.object.common.constants import CommonOperationID
from app.resource.rpc.hw_agent_rpc import update_oracle_snmp_trap
from app.resource.schemas.host_models import (
    Host,
    AsmInfo,
    ClusterHost,
    HostDetail, HostCreateInfo, MigrationReq)
from app.resource.service.host import host_res_service as service
from app.resource.service.host.host_migrate_service import HostMigrateObjectService, get_migrate_host_ip_by_params

host_router = async_route()
log = get_logger(__name__)


def _resolve_user_info(token: str = Header(..., alias="X-Auth-Token", title="X-Auth-Token",
                                           description="访问令牌")) -> dict:
    return get_user_info_from_token(token)


@exter_attack
@host_router.post(
    '/resource/host/',
    status_code=200,
    response_model=Host,
    tags=['debug'],
    summary="注册/更新/卸载主机",
    response_description="The response is the created host",
    responses={404: {"detail": "Not found"}},
)
def create_host(host: HostCreateInfo):
    log.info(f'[Add Host]: ip:{host.ip}')
    # 把linux操作系统统一展示
    if host.os_type in LinuxOsTypeEnum.__members__.values():
        host.os_type = "linux"
    env = service.add_host(host)
    if env:
        return Host(**env)
    return Host()


@exter_attack
@host_router.get(
    '/resource/host/',
    status_code=200,
    response_model=BasePage[HostDetail],
    summary="分页查询主机信息列表",
    response_description="The response is the list of all host names and IDs available",
    responses={500: {"detail": "Internal error"}},
)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR},
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.AGENT, operation=OperationTypeEnum.QUERY,
                              target="uuid")
)
def page_query_hosts(
        page_no: int = Query(..., ge=0, le=10000, description="分页页面编码"),
        page_size: int = Query(..., ge=0, le=200, description="分页数据条数"),
        type_of_app: str = Query(None, max_length=64, description="主机应用类型"),
        uuid: str = Query(None, min_length=1, max_length=64, description="主机ID"),
        conditions: str = Query(None, min_length=1, max_length=1024, description="条件参数"),
        token: str = Header(..., min_length=1, max_length=10000, alias="X-Auth-Token", title="X-Auth-Token",
                            description="访问令牌")):
    current_user_info = get_user_info_from_token(token)
    clear(token)
    result = service.page_query_host(
        page_no, page_size, type_of_app, uuid, current_user_info, conditions)
    return result


@exter_attack
# 获取可以升级的agent信息
@host_router.get(
    '/resource/hosts/upgradeable-versions',
    status_code=200,
    response_model=List,
    summary="查询主机可升级的版本",
    response_description="The response is the info about a can update host",
    responses={404: {"detail": "Not found"}},
)
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR})
def get_update_agent_host(
        db: Session = async_depend(db_config.get_db_session),
        host_uuids: List[str] = Query(..., description="查询版本的uuid列表", min_length=1)):
    data = service.get_can_update_agent_host(uuids=host_uuids, session=db)
    return data


@exter_attack
@host_router.get(
    '/resource/host/{host_id}',
    status_code=200,
    response_model=Optional[Host],
    summary="根据主机ID查询主机信息",
    response_description="The response is the info about a host - TBD",
    responses={404: {"detail": "Not found"}},
)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR}, resources="resource:host_id",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.AGENT, operation=OperationTypeEnum.QUERY,
                              target="host_id")
)
def get_host(host_id: str = Path(..., min_length=1, max_length=64, description="主机ID")):
    log.debug(f'[Resource Host]: {host_id}')
    return service.get_host_by_id(host_id)


@exter_attack
def get_host_info_by_host_id(params):
    default_host_info = None
    host_id = params.get("host_id")
    url = f'/v1/internal/resource/host/{host_id}'
    log.info(f'[Invoke Api]: create jon, request url: {url}')
    response = ProtectionServiceHttpsClient().request("GET", url)
    if response.status == HTTPStatus.OK:
        return parse_response_data(response.data)
    else:
        log.error(f'[Failed query resource]: resource id is host_id: {host_id}')
    return default_host_info


def get_host_name_ip_by_host_id(params):
    host_info = get_host_info_by_host_id(params)
    if host_info is not None:
        name = host_info.get('name')
        return str(name)
    else:
        return "--"


def get_host_ip_by_host_id(params):
    host_info = get_host_info_by_host_id(params)
    if host_info is not None:
        endpoint = host_info.get('endpoint')
        return str(endpoint)
    else:
        return "--"


@exter_attack
# 操作日志:用户（{0}:{1}）移除主机（名称:{2},IP:{3}）。
@host_router.delete(
    '/resource/host/{host_id}',
    status_code=204,
    summary="根据主机ID删除主机",
    response_description="Deleted",
    responses={404: {"detail": "Not found"}},
)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="resource:host_id",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.AGENT, operation=OperationTypeEnum.DELETE,
                              auth_operation_list={AuthOperationEnum.MANAGE_RESOURCE}, target="host_id")
)
@operation_log(
    name=CommonOperationID.REMOVE_HOST,
    target="@Resource",
    detail=(get_host_name_ip_by_host_id, get_host_ip_by_host_id)
)
def delete_host(host_id: str = Path(..., min_length=1, max_length=64, description="主机ID"),
                db: Session = async_depend(db_config.get_db_session)):
    log.debug(f'[DELETE Host]: {host_id}')
    host = service.get_host_by_id(host_id)
    if host is None:
        raise EmeiStorBizException(CommonErrorCodes.STATUS_ERROR,
                                   message=f"Cannot delete because host not exist.")
    service.delete_host(host, db)


@exter_attack
@host_router.get(
    '/internal/resource/host/{host_id}/asm-info',
    status_code=200,
    summary="根据主机ID查询主机ASM信息列表",
    response_model=List[AsmInfo],
    response_description="The response is the newly created fileset",
    responses={500: {"detail": "Internal error"}},
)
def asm_info(host_id: str = Path(..., description="主机ID")):
    return service.asm_info(host_id)


# 映射认证方式,1:OS认证,0:数据库认证
def get_auth_type_map(params):
    default_auth_type_map = None
    asm_auth_req = params.get("asm_auth_req")
    auth_type = asm_auth_req.auth_type
    if auth_type == 1:
        return "OS认证"
    if auth_type == 0:
        return "数据库认证"
    return default_auth_type_map


@exter_attack
@host_router.get(
    '/resource/host/{cluster_id}/nodes',
    status_code=200,
    summary="根据集群ID查询集群的主机信息列表",
    response_model=List[ClusterHost],
    response_description="The response is the cluster Info for a host",
    responses={500: {"detail": "Internal error"}},
)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR}, resources="resource:cluster_id",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.QUERY,
                              target="cluster_id")
)
# cluster_id: the uuid of cluster's info.
def host_cluster_info(cluster_id: str = Path(..., description="集群ID", max_length=64),
                      db: Session = async_depend(db_config.get_db_session)):
    log.debug(f'[Host Nodes] cluster_id: {cluster_id}')
    return service.get_host_cluster_info(cluster_id, db)


@exter_attack
@host_router.put(
    '/internal/resource/host/{host_id}/refresh_host',
    status_code=200,
    summary="刷新主机信息【内部接口】",
    response_model=List[str],
    responses={500: {"detail": "Internal error"}},
)
def host_refresh_host_cluster(host_id: str = Path(..., description="集群/主机ID"),
                              db_name: str = Query(
                                  None, description="数据库名称", example="oracle"),
                              is_valid: bool = Query(
                                  None, description="是否排除失效数据库", example="oracle"),
                              is_clear_protection_object: bool = Query(
                                  None, description="是否清除保护对象", example="oracle"),
                              db: Session = async_depend(db_config.get_db_session)):
    log.debug(f'[Refresh Host] host_id: {host_id}')
    if is_valid is None:
        is_valid = False
    return service.refresh_cluster_host_info(host_id, db, db_name, is_valid, is_clear_protection_object)


@exter_attack
@host_router.put(
    '/resource/host/{host_id}/action/modify',
    status_code=200,
    summary="修改主机名名称",
    response_model=List[str],
    responses={500: {"detail": "Internal error"}},
)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="resource:host_id",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.MODIFY,
                              auth_operation_list={AuthOperationEnum.MANAGE_RESOURCE}, target="host_id")
)
@operation_log(
    name=CommonOperationID.MODIFY_HOSTNAME,
    target="@Resource",
    detail=('host_id!resource.path', 'host_name')
)
def modify_host(host_id: str = Path(..., description="主机ID", min_length=1, max_length=64),
                host_name: str = Query(None, min_length=1, max_length=64, description="主机名称",
                                       example="localhost")):
    return service.manual_modify_host_name(host_id, host_name)


@exter_attack
@host_router.get(
    '/internal/resource/host/{host_id}',
    status_code=200,
    response_model=Optional[Host],
    summary="根据主机ID查询主机信息【内部接口】",
    response_description="The response is the info about a host - TBD",
    responses={404: {"detail": "Not found"}},
)
def internal_get_host(host_id: str = Path(..., min_length=1, max_length=64, description="主机ID")):
    log.debug(f'[Internal Resource Host] host_id: {host_id}')
    return service.get_host_by_id(host_id)


@exter_attack
@host_router.post(
    '/resource/host/{host_id}/action/sync-snmp-conf',
    status_code=200,
    summary="同步Trap信息到Oracle主机",
    response_description="The response is the newly created fileset",
    responses={500: {"detail": "Internal error"}},
)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="resource:host_id",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.MODIFY,
                              auth_operation_list={AuthOperationEnum.MANAGE_RESOURCE}, target="host_id")
)
@operation_log(
    name=CommonOperationID.SYNC_SNMP_TRAP,
    target="@Resource",
    detail=(get_host_ip_by_host_id)
)
def synchronize_snmp_to_host(host_id: str = Path(..., description="主机ID", min_length=1, max_length=64)):
    log.info(f"Start to synchronize snmp to host.")
    service.check_host_resource_if_san_client(host_id)
    ip, date = service.manual_synchronize_snmp_to_agent(host_id)
    return update_oracle_snmp_trap(ip, date)


@exter_attack
@host_router.post(
    '/resource/host/action/migrate',
    status_code=200,
    summary="迁移主机到目标集群",
    response_description="Migrating Hosts to the Target Cluster",
    responses={500: {"detail": "Internal error"}},
)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="resource:host_id",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.MODIFY,
                              auth_operation_list={AuthOperationEnum.MANAGE_RESOURCE}, target="host_id")
)
@operation_log(
    name=CommonOperationID.MIGRATE_HOST,
    target="@Resource",
    detail=(get_migrate_host_ip_by_params)
)
def host_migrate(
        user_info: dict = async_depend(_resolve_user_info),
        host_migrate_req: MigrationReq = Body(..., description="迁移主机请求")):
    log.debug("[host migrate]")
    log.info(f"check_host_resource_if_configured_lan_free:{host_migrate_req.host_migrate_req[0].host_id}.")
    service.check_host_resource_if_configured_lan_free(host_migrate_req.host_migrate_req[0].host_id)
    result = HostMigrateObjectService.create_migrate_object(user_info.get("user-id"), host_migrate_req)
    return result
