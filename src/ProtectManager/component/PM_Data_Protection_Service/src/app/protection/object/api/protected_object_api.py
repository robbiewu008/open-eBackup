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
import uuid
from typing import List, Optional

from fastapi import Header, Body, Query, Path
from sqlalchemy.orm import Session

import app.protection.object.db as curd
from app.common import logger
from app.common.auth.check_ath import CheckAuthModel
from app.common.concurrency import async_route, async_depend
from app.common.enums.rbac_enum import ResourceSetTypeEnum, OperationTypeEnum, AuthOperationEnum
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import PolicyActionEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException, IllegalParamException
from app.common.exter_attack import exter_attack
from app.common.log.operation_log import operation_log
from app.common.schemas.common_schemas import BasePage
from app.common.security.jwt_utils import get_user_info_from_token
from app.common.security.right_control import right_control
from app.common.security.role_dict import RoleEnum
from app.copy_catalog.service.curd.copy_query_service import check_copy_name_valid
from app.protection.object.common import db_config
from app.protection.object.common.constants import CommonOperationID
from app.protection.object.schemas.protected_copy_object import SlaResourceQuantityRelationship
from app.protection.object.schemas.protected_object import CurrentManualBackupRequest, ProtectedObjectQuery, \
    ProtectedObjectId, ModifyProtectionSubmitReq, BatchProtectionSubmitReq, ManualBackupReq, ProtectedObjectTime, \
    BatchOperationReq, ComplianceUpdate, ProtectedObjectQueryResponse, ProtectedObjectQueryRequest, \
    ProtectedObjectSlaCompliance, ModifyProtectObjExtInfoReq, UpdateSelfLearningProgressReq
from app.protection.object.service.batch_protection_service import BatchProtectionService
from app.protection.object.service.projected_object_service import ProtectedObjectService, \
    get_batch_protection_log_data, get_resource_name_and_id, get_manual_backup_log_data, \
    get_modify_protection_log_data, get_modify_cyber_protection_log_data, get_create_cyber_protection_log_data, \
    get_batch_create_cyber_protection_log_data, get_manual_backup_cyber_log_data, \
    get_protection_cyber_log_data
from app.resource.service.common import protect_obj_service, resource_service
from app.resource.service.common.protect_obj_service import get_protect_obj, get_next_time
from app.resource.service.common.user_domain_service import get_domain_id_by_user_id

api = async_route()
log = logger.get_logger(__name__)


def _resolve_user_info(token: str = Header(..., alias="X-Auth-Token", title="X-Auth-Token",
                                           description="访问令牌")) -> dict:
    return get_user_info_from_token(token)


@exter_attack
@api.get("/protected-objects", status_code=200,
         response_model=BasePage[ProtectedObjectQueryResponse],
         summary="分页查询SLA关联资源信息列表")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR}, resources=("sla:sla_id"),
    check_auth=CheckAuthModel(
        resource_set_type=ResourceSetTypeEnum.SLA, operation=OperationTypeEnum.QUERY,
        auth_operation_list={AuthOperationEnum.BACKUP, AuthOperationEnum.REPLICATION, AuthOperationEnum.ARCHIVE},
        target="sla_id")
)
def page_query(
        db: Session = async_depend(db_config.get_db_session),
        user_info: dict = async_depend(_resolve_user_info),
        name: str = Query(None, description="资源名称，支持模糊查询", max_length=128),
        sub_type: List[ResourceSubTypeEnum] = Query(
            None, description="SLA类型，支持筛选过滤"),
        path: str = Query(None, description="资源位置，支持模糊查询", max_length=255),
        sla_compliance: List[bool] = Query(None, description="SLA遵从度，支持筛选过滤"),
        sla_id: str = Query(..., description="SLA的ID", max_length=64),
        page_no: int = Query(..., description="分页页面编码", ge=0, le=10000),
        page_size: int = Query(..., description="分页数据条数", ge=0, le=200)):
    page_req = ProtectedObjectQueryRequest(domain_id=user_info.get("domain-id"), name=name, sub_type=sub_type,
                                           path=path, sla_compliance=sla_compliance, sla_id=sla_id, page_no=page_no,
                                           page_size=page_size)
    return curd.projected_object.page_query(db=db, page_req=page_req)


@exter_attack
@api.get(
    "/internal/protected-objects", status_code=200,
    response_model=BasePage[ProtectedObjectQueryResponse],
    summary="分页查询SLA关联资源信息列表【内部接口】"
)
def internal_page_query(
        db: Session = async_depend(db_config.get_db_session),
        name: str = Query(None, description="资源名称，支持模糊查询"),
        sub_type: List[ResourceSubTypeEnum] = Query(
            None, description="SLA类型，支持筛选过滤"),
        path: str = Query(None, description="资源位置，支持模糊查询"),
        sla_compliance: List[bool] = Query(None, description="SLA遵从度，支持筛选过滤"),
        sla_id: str = Query(..., description="SLA的ID"),
        page_no: int = Query(..., description="分页页面编码"),
        page_size: int = Query(..., description="分页数据条数")):
    page_req = ProtectedObjectQueryRequest(name=name, sub_type=sub_type, path=path,
                                           sla_compliance=sla_compliance, sla_id=sla_id, page_no=page_no,
                                           page_size=page_size)
    return curd.projected_object.page_query(db=db, page_req=page_req)


@exter_attack
@api.post("/protected-objects", status_code=200, summary="创建保护对象", description="创建保护对象，绑定SLA",
          response_model=ProtectedObjectId)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    resources=("resource:create_req.resource_id, create_req.ext_parameters.proxy_id", "sla:create_req.sla_id"),
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.MODIFY,
                              auth_operation_list={AuthOperationEnum.BACKUP, AuthOperationEnum.REPLICATION,
                                                   AuthOperationEnum.ARCHIVE},
                              target="create_req.resource_id")
)
@operation_log(
    name=CommonOperationID.CREATED_PROTECTION,
    target="@Protection",
    detail=('create_req.resource_id!resource.name')
)
def create(
        user_info: dict = async_depend(_resolve_user_info),
        db: Session = async_depend(db_config.get_db_session),
        create_req: ModifyProtectionSubmitReq = Body(..., description="创建保护对象请求")):
    log.info(f'create_req={create_req}')
    BatchProtectionService.check_auth(user_info.get("domain-id"), create_req.sla_id)
    obj_id = ProtectedObjectService.create_projected_object(
        session=db, create_req=create_req)
    return ProtectedObjectId(uuid=obj_id)


@exter_attack
@api.post("/internal/protected-objects", status_code=200, summary="创建保护对象", description="创建保护对象，绑定SLA",
          response_model=ProtectedObjectId)
def internal_create(
        db: Session = async_depend(db_config.get_db_session),
        create_req: ModifyProtectionSubmitReq = Body(..., description="创建保护对象请求")):
    log.info(f'Start to create protected_object for resource {create_req.resource_id} with sla {create_req.sla_id}.')

    obj_id = ProtectedObjectService.create_projected_object(session=db, create_req=create_req)
    return ProtectedObjectId(uuid=obj_id)


@exter_attack
@api.post("/protected-objects-cyber", status_code=200, summary="创建保护对象", description="创建保护对象，绑定SLA",
          response_model=ProtectedObjectId)
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
               resources=("resource:create_req.resource_id, create_req.ext_parameters.proxy_id",
                          "sla:create_req.sla_id"))
@operation_log(
    name=CommonOperationID.CREATED_PROTECTION_CYBER,
    target="@Protection",
    detail=(get_create_cyber_protection_log_data)
)
def create_cyber(
        db: Session = async_depend(db_config.get_db_session),
        create_req: ModifyProtectionSubmitReq = Body(..., description="创建保护对象请求")):
    log.info(f'create_req={create_req}')
    obj_id = ProtectedObjectService.create_projected_object(
        session=db, create_req=create_req)
    return ProtectedObjectId(uuid=obj_id)


@exter_attack
@api.post("/protected-objects/batch", status_code=200, summary="批量创建保护对象",
          description="批量创建保护对象，绑定SLA")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    resources=("resource:batch_create_req.resources.*.resource_id, batch_create_req.ext_parameters.proxy_id",
               "sla:create_req.sla_id"),
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.MODIFY,
                              auth_operation_list={AuthOperationEnum.BACKUP, AuthOperationEnum.REPLICATION,
                                                   AuthOperationEnum.ARCHIVE},
                              target="batch_create_req.resources.*.resource_id")
)
@operation_log(
    name=CommonOperationID.CREATED_PROTECTION,
    target="@Protection",
    detail=(get_batch_protection_log_data)
)
def batch_create(
        user_info: dict = async_depend(_resolve_user_info),
        batch_create_req: BatchProtectionSubmitReq = Body(..., description="批量创建保护对象请求")):
    resource_ids = [str(resource.resource_id) for resource in batch_create_req.resources]
    BatchProtectionService.check_is_resource_group(resource_ids=resource_ids)
    BatchProtectionService.check_auth(user_info.get("domain-id"), batch_create_req.sla_id)
    BatchProtectionService.submit_batch_protection_task(user_info.get("user-id"), batch_create_req)


@exter_attack
@api.post("/protected-objects-cyber/batch", status_code=200, summary="批量创建保护对象",
          description="批量创建保护对象，绑定SLA")
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
               resources=(
                       "resource:batch_create_req.resources.*.resource_id, batch_create_req.ext_parameters.proxy_id",
                       "sla:create_req.sla_id"))
@operation_log(
    name=CommonOperationID.CREATED_PROTECTION_CYBER,
    target="@Protection",
    detail=(get_batch_create_cyber_protection_log_data)
)
def batch_create_cyber(
        user_info: dict = async_depend(_resolve_user_info),
        batch_create_req: BatchProtectionSubmitReq = Body(..., description="批量创建保护对象请求")):
    BatchProtectionService.submit_batch_protection_task(user_info.get("user-id"), batch_create_req)


@exter_attack
@api.post("/internal/protected-objects/batch", status_code=200, summary="批量创建保护对象",
          description="批量创建保护对象，绑定SLA")
def internal_batch_create(user_id: str = Query(..., description="用户ID", min_length=1, max_length=64),
                          batch_create_req: BatchProtectionSubmitReq = Body(..., description="批量创建保护对象请求")):
    return BatchProtectionService.submit_batch_protection_task(user_id, batch_create_req)


@exter_attack
@api.get("/internal/protected-objects/count", status_code=200,
         summary="根据SLA的ID查询关联资源数量【内部接口】")
def count_protection_object(
        db: Session = async_depend(db_config.get_db_session),
        sla_id: uuid.UUID = Query(..., description="SLA的ID"),
        domain_id: str = Query(None, description="用户域ID")):
    return ProtectedObjectService.count_by_sla_id(session=db, sla_id=str(sla_id), domain_id=domain_id)


@exter_attack
@api.get("/internal/protected-objects/{resource_id}", status_code=200, response_model=Optional[ProtectedObjectQuery],
         summary="根据资源ID查询保护对象信息【内部接口】")
def query_protection_object(
        db: Session = async_depend(db_config.get_db_session),
        resource_id: str = Path(..., description="保护对象资源ID")):
    obj = ProtectedObjectService.query_obj(session=db, resource_id=resource_id)
    if obj and isinstance(obj.ext_parameters, dict):
        obj.ext_parameters = json.dumps(obj.ext_parameters)
    return obj


@exter_attack
@api.put("/internal/protected-objects/action/sync-sla", status_code=200, summary="保护对象同步SLA信息变更【内部接口】")
def synchronize_sla_change(db: Session = async_depend(db_config.get_db_session),
                           sla_id: str = Body(..., description="SLA的ID", embed=True)):
    return ProtectedObjectService.sync_sla_info_change(session=db, sla_id=sla_id)


@exter_attack
@api.delete("/protected-objects", status_code=200,
            summary="批量移除保护")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="resource:req.resource_ids",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.MODIFY,
                              auth_operation_list={AuthOperationEnum.BACKUP, AuthOperationEnum.REPLICATION,
                                                   AuthOperationEnum.ARCHIVE},
                              target="req.resource_ids")
)
@operation_log(
    name=CommonOperationID.REMOVE_PROTECTION,
    target="@Protection",
    detail=(get_resource_name_and_id)
)
def delete(
        db: Session = async_depend(db_config.get_db_session),
        req: BatchOperationReq = Body(..., description="批量移除保护请求")):
    BatchProtectionService.check_is_resource_group(
        resource_ids=req.resource_ids)
    BatchProtectionService.check_resource_status(
        session=db, resource_ids=req.resource_ids)
    BatchProtectionService.batch_remove_protection(
        session=db, resource_ids=req.resource_ids)


@exter_attack
@api.delete("/protected-objects-cyber", status_code=200,
            summary="移除保护")
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources=("resource:req.resource_ids"))
@operation_log(
    name=CommonOperationID.REMOVE_PROTECTION_CYBER,
    target="@Protection",
    detail=(get_protection_cyber_log_data)
)
def delete_cyber(
        db: Session = async_depend(db_config.get_db_session),
        req: BatchOperationReq = Body(..., description="移除保护请求")):
    BatchProtectionService.check_resource_status(
        session=db, resource_ids=req.resource_ids)
    BatchProtectionService.batch_remove_protection(
        session=db, resource_ids=req.resource_ids)


@exter_attack
@api.delete("/internal/protected-objects", status_code=200,
            summary="批量移除保护")
def internal_delete(
        db: Session = async_depend(db_config.get_db_session),
        req: BatchOperationReq = Body(..., description="批量移除保护请求")):
    BatchProtectionService.check_resource_status(
        session=db, resource_ids=req.resource_ids)
    BatchProtectionService.batch_remove_protection(
        session=db, resource_ids=req.resource_ids, is_resource_group=req.is_resource_group)


@exter_attack
@api.put("/protected-objects", status_code=200, summary="修改保护")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    resources=("resource:update_req.resource_id, update_req.ext_parameters.proxy_id", "sla:update_req.sla_id"),
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.MODIFY,
                              auth_operation_list={AuthOperationEnum.BACKUP, AuthOperationEnum.REPLICATION,
                                                   AuthOperationEnum.ARCHIVE},
                              target="submit_req.resource_id")
)
@operation_log(
    name=CommonOperationID.MODIFY_PROTECTION,
    target="@Protection",
    detail=(get_modify_protection_log_data),
)
def modify(
        user_info: dict = async_depend(_resolve_user_info),
        submit_req: ModifyProtectionSubmitReq = Body(..., description="修改保护对象参数")):
    resource_ids = [submit_req.resource_id]
    BatchProtectionService.check_is_resource_group(
        resource_ids=resource_ids)
    BatchProtectionService.check_auth(user_info.get("domain-id"), submit_req.sla_id)
    obj_id = BatchProtectionService.modify_protection_task_submit(user_info.get("user-id"), submit_req)
    return ProtectedObjectId(uuid=obj_id)


@exter_attack
@api.put("/protected-objects-cyber", status_code=200, summary="修改保护")
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
               resources=("resource:update_req.resource_id, update_req.ext_parameters.proxy_id",
                          "sla:update_req.sla_id"))
@operation_log(
    name=CommonOperationID.MODIFY_PROTECTION_CYBER,
    target="@Protection",
    detail=(get_modify_cyber_protection_log_data),
)
def modify_cyber(
        user_info: dict = async_depend(_resolve_user_info),
        submit_req: ModifyProtectionSubmitReq = Body(..., description="修改保护对象参数")):
    obj_id = BatchProtectionService.modify_protection_task_submit(user_info.get("user-id"), submit_req)
    return ProtectedObjectId(uuid=obj_id)


@exter_attack
@api.put("/internal/protected-objects", status_code=200, summary="修改保护")
def internal_modify(
        user_id: str = Query(None, description="用户id", max_length=64),
        submit_req: ModifyProtectionSubmitReq = Body(..., description="修改保护对象参数")):
    log.info(f"Internal modify protected objects, sla id: {submit_req.sla_id}, resource id: {submit_req.resource_id}")
    return BatchProtectionService.modify_protection_task_submit(user_id, submit_req)


@exter_attack
@api.put("/protected-objects/status/action/activate", status_code=200, summary="批量激活保护")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources=("resource:req.resource_ids"),
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.MODIFY,
                              auth_operation_list={AuthOperationEnum.BACKUP, AuthOperationEnum.REPLICATION,
                                                   AuthOperationEnum.ARCHIVE},
                              target="req.resource_ids")
)
@operation_log(
    name=CommonOperationID.ACTIVATE_PROTECTION,
    target="@Protection",
    detail=(get_resource_name_and_id),
)
def active(
        db: Session = async_depend(db_config.get_db_session),
        req: BatchOperationReq = Body(..., description="激活批量操作请求")):
    BatchProtectionService.batch_activate(
        session=db, resource_ids=req.resource_ids)
    return ""


@exter_attack
@api.put("/protected-objects/status/action/activate-cyber", status_code=200, summary="激活保护")
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources=("resource:req.resource_ids"))
@operation_log(
    name=CommonOperationID.ACTIVATE_PROTECTION_CYBER,
    target="@Protection",
    detail=(get_protection_cyber_log_data),
)
def active_cyber(
        database: Session = async_depend(db_config.get_db_session),
        req: BatchOperationReq = Body(..., description="激活操作请求")):
    BatchProtectionService.batch_activate(
        session=database, resource_ids=req.resource_ids)
    return ""


@exter_attack
@api.put("/internal/protected-objects/status/action/activate", status_code=200, summary="批量激活保护")
def internal_activate(
        database: Session = async_depend(db_config.get_db_session),
        req: BatchOperationReq = Body(..., description="激活批量操作请求")):
    BatchProtectionService.batch_activate(
        session=database, resource_ids=req.resource_ids)


@exter_attack
@api.put("/protected-objects/status/action/deactivate", status_code=200,
         summary="批量取消激活保护")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources=("resource:req.resource_ids"),
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.MODIFY,
                              auth_operation_list={AuthOperationEnum.BACKUP, AuthOperationEnum.REPLICATION,
                                                   AuthOperationEnum.ARCHIVE},
                              target="req.resource_ids")
)
@operation_log(
    name=CommonOperationID.DEACTIVATE_PROTECTION,
    target="@Protection",
    detail=(get_resource_name_and_id),
)
def deactivate(
        db_session: Session = async_depend(db_config.get_db_session),
        req: BatchOperationReq = Body(..., description="取消激活批量操作请求")):
    BatchProtectionService.batch_deactivate(
        session=db_session, resource_ids=req.resource_ids)
    return ""


@exter_attack
@api.put("/protected-objects/status/action/deactivate-cyber", status_code=200,
         summary="禁用保护")
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources=("resource:req.resource_ids"))
@operation_log(
    name=CommonOperationID.DEACTIVATE_PROTECTION_CYBER,
    target="@Protection",
    detail=(get_protection_cyber_log_data),
)
def deactivate_cyber(
        db_session: Session = async_depend(db_config.get_db_session),
        req: BatchOperationReq = Body(..., description="取消激活操作请求")):
    BatchProtectionService.batch_deactivate(
        session=db_session, resource_ids=req.resource_ids)
    return ""


@exter_attack
@api.put("/internal/protected-objects/status/action/deactivate", status_code=200,
         summary="批量取消激活保护")
def internal_deactivate(
        db_session: Session = async_depend(db_config.get_db_session),
        req: BatchOperationReq = Body(..., description="取消激活批量操作请求")):
    BatchProtectionService.batch_deactivate(
        session=db_session, resource_ids=req.resource_ids)


@exter_attack
@api.put("/internal/protected-objects/status/action/activate-resource-group", status_code=200,
         summary="虚拟机组批量激活保护")
def active_resource_group(
        db: Session = async_depend(db_config.get_db_session),
        req: BatchOperationReq = Body(..., description="虚拟机组激活批量操作请求")):
    BatchProtectionService.batch_activate_resource_group(
        session=db, resource_ids=req.resource_ids, is_resource_group=req.is_resource_group)
    if req.is_resource_group:
        for resource_id in req.resource_ids:
            # 从保护对象表中查询对应资源组id的
            resource_id_list = resource_service.query_group_resources_by_group_id(resource_id)
            BatchProtectionService.batch_activate_resource_group(
                session=db, resource_ids=resource_id_list, is_resource_group=False)
    return ""


@exter_attack
@api.put("/internal/protected-objects/status/action/deactivate-resource-group", status_code=200,
         summary="虚拟机组批量取消激活保护")
def deactivate_resource_group(
        db_session: Session = async_depend(db_config.get_db_session),
        req: BatchOperationReq = Body(..., description="虚拟机组取消激活批量操作请求")):
    BatchProtectionService.batch_deactivate_resource_group(
        session=db_session, resource_ids=req.resource_ids, is_resource_group=req.is_resource_group)
    if req.is_resource_group:
        for resource_id in req.resource_ids:
            resource_id_list = resource_service.query_group_resources_by_group_id(resource_id)
            BatchProtectionService.batch_deactivate_resource_group(
                session=db_session, resource_ids=resource_id_list, is_resource_group=False)
    return ""


def check_tidb_back_type(backup_req, res):
    resource_type = res.sub_type
    if resource_type == ResourceSubTypeEnum.TiDB_CLUSTER.value:
        if backup_req.action in [PolicyActionEnum.cumulative_increment.value,
                                 PolicyActionEnum.difference_increment.value,
                                 PolicyActionEnum.permanent_increment.value]:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["action"])
    if resource_type in [ResourceSubTypeEnum.TiDB_DATABASE.value, ResourceSubTypeEnum.TiDB_TABLE.value]:
        if backup_req.action in [PolicyActionEnum.log.value, PolicyActionEnum.cumulative_increment.value,
                                 PolicyActionEnum.difference_increment.value,
                                 PolicyActionEnum.permanent_increment.value]:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["action"])


def check_antdb_back_type(backup_req, res):
    resource_type = res.sub_type
    if resource_type in [ResourceSubTypeEnum.AntDBClusterInstance.value, ResourceSubTypeEnum.AntDBInstance.value]:
        if backup_req.action in [PolicyActionEnum.cumulative_increment.value,
                                 PolicyActionEnum.difference_increment.value,
                                 PolicyActionEnum.permanent_increment.value]:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["action"])


def check_tdsql_back_type(backup_req, res):
    resource_type = res.sub_type
    if resource_type == ResourceSubTypeEnum.TDSQL_CLUSTER_GROUP.value:
        if backup_req.action in [PolicyActionEnum.cumulative_increment.value,
                                 PolicyActionEnum.difference_increment.value,
                                 PolicyActionEnum.permanent_increment.value]:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["action"])
    if resource_type == ResourceSubTypeEnum.TDSQL_CLUSTER_INSTANCE.value:
        if backup_req.action in [PolicyActionEnum.cumulative_increment.value,
                                 PolicyActionEnum.permanent_increment.value]:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["action"])


def check_oceanbase_back_type(backup_req, res):
    resource_type = res.sub_type
    if resource_type == ResourceSubTypeEnum.OCEANBASE_CLUSTER.value:
        if backup_req.action in [PolicyActionEnum.cumulative_increment.value,
                                 PolicyActionEnum.permanent_increment.value]:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["action"])
    if resource_type == ResourceSubTypeEnum.OCEANBASE_TENANT.value:
        if backup_req.action in [PolicyActionEnum.log.value, PolicyActionEnum.cumulative_increment.value,
                                 PolicyActionEnum.difference_increment.value,
                                 PolicyActionEnum.permanent_increment.value]:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["action"])


@exter_attack
@api.post("/protected-objects/{resource_id}/action/backup", status_code=200, summary="手动备份资源")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    resources=("resource:resource_id", "sla:backup_req.sla_id"),
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.MODIFY,
                              auth_operation_list={AuthOperationEnum.BACKUP}, target="resource_id")
)
@operation_log(
    name=CommonOperationID.MANUALLY_BACK,
    target="@Protection",
    detail=(get_manual_backup_log_data, '#result'),
)
def manual_backup(
        db_session: Session = async_depend(db_config.get_db_session),
        user_info: dict = async_depend(_resolve_user_info),
        resource_id: str = Path(..., description="保护对象资源ID", max_length=64),
        backup_req: ManualBackupReq = Body(..., description="手动备份资源请求")):
    log.info(f'resource_id={resource_id}, backup_req={backup_req}')

    check_copy_name_valid(backup_req.copy_name)
    if backup_req.is_resource_group:
        resource_group = resource_service.query_resource_group_by_id(resource_id)
        if not resource_group:
            raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST,
                                       message=f"resource group [{resource_id}] is not existed.")
    else:
        res = resource_service.query_resource_by_id(resource_id)
        if not res:
            raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST,
                                       message=f"resource [{resource_id}] is not existed")
        check_tidb_back_type(backup_req, res)
        check_antdb_back_type(backup_req, res)
        check_oceanbase_back_type(backup_req, res)
        check_tdsql_back_type(backup_req, res)

    user_backup_req = CurrentManualBackupRequest.parse_obj(backup_req.dict())
    user_backup_req.user_id = user_info.get("user-id")
    backup_job_ids = ProtectedObjectService.manual_backup(
        session=db_session, resource_id=resource_id, backup_req=user_backup_req)
    return backup_job_ids


@exter_attack
@api.post("/protected-objects/{resource_id}/action/backup-cyber", status_code=200, summary="手动备份资源")
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
               resources=("resource:resource_id", "sla:backup_req.sla_id"))
@operation_log(
    name=CommonOperationID.MANUALLY_BACK_CYBER,
    target="@Protection",
    detail=(get_manual_backup_cyber_log_data, '#result'),
)
def manual_backup_cyber(
        db_session: Session = async_depend(db_config.get_db_session),
        user_info: dict = async_depend(_resolve_user_info),
        resource_id: str = Path(..., description="保护对象资源ID", max_length=64),
        backup_req: ManualBackupReq = Body(..., description="手动备份资源请求")):
    log.info(f'resource_id={resource_id}, backup_req={backup_req}')

    check_copy_name_valid(backup_req.copy_name)
    res = resource_service.query_resource_by_id(resource_id)
    if not res:
        raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST,
                                   message=f"resource [{resource_id}] is not existed")
    user_backup_req = CurrentManualBackupRequest.parse_obj(backup_req.dict())
    user_backup_req.user_id = user_info.get("user-id")
    backup_job_ids = ProtectedObjectService.manual_backup(
        session=db_session, resource_id=resource_id, backup_req=user_backup_req)
    return backup_job_ids


@exter_attack
@api.post("/internal/protected-objects/{resource_id}/action/backup", status_code=200, summary="手动备份资源")
def internal_manual_backup(
        db_session: Session = async_depend(db_config.get_db_session),
        user_id: str = Query(None, description="用户id", max_length=64),
        resource_id: str = Path(..., description="保护对象资源ID", max_length=64),
        backup_req: ManualBackupReq = Body(..., description="手动备份资源请求")):
    log.info(f'internal execute manual backup, resource_id={resource_id}, backup_req={backup_req}')

    check_copy_name_valid(backup_req.copy_name)
    if backup_req.is_resource_group:
        resource_group = resource_service.query_resource_group_by_id(resource_id)
        if not resource_group:
            raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST,
                                       message=f"resource group [{resource_id}] is not existed.")
    else:
        res = resource_service.query_resource_by_id(resource_id)
        if not res:
            raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST,
                                       message=f"resource [{resource_id}] is not existed")
    user_backup_req = CurrentManualBackupRequest.parse_obj(backup_req.dict())
    user_backup_req.user_id = user_id
    backup_job_ids = ProtectedObjectService.manual_backup(
        session=db_session, resource_id=resource_id, backup_req=user_backup_req)
    return backup_job_ids


@exter_attack
@api.put("/internal/protected-objects/compliance", summary="更新保护对象遵从度【内部接口】")
def update_compliance(
        database: Session = async_depend(db_config.get_db_session),
        update_req: ComplianceUpdate = Body(..., description="更新保护对象遵从度请求")):
    ProtectedObjectService.update_compliance(session=database, resource_id=update_req.resource_id,
                                             compliance=update_req.compliance)


@exter_attack
@api.put(
    "/internal/protected-objects/{resource_id}/action/update-last-backup-time",
    status_code=200, summary="更新保护对象的最新备份时间【内部接口】")
def synchronize_time(db: Session = async_depend(db_config.get_db_session),
                     resource_id: str = Path(..., description="保护对象资源ID")):
    return protect_obj_service.sync_time(db, resource_id)


@exter_attack
@api.get(
    "/protected-objects/{resource_id}/backup-time",
    status_code=200, summary="查询保护对象的备份时间信息",
    response_model=ProtectedObjectTime)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR}, resources=("resource:resource_id")
)
def query_protection_time(db: Session = async_depend(db_config.get_db_session),
                          token: str = Header(..., alias="X-Auth-Token", title="X-Auth-Token", description="访问令牌"),
                          resource_id: str = Path(..., description="保护对象资源ID", max_length=64)):
    user_info = get_user_info_from_token(token)
    domain_id = get_domain_id_by_user_id(user_info.get("user-id"))
    protected_object = get_protect_obj(db, resource_id, domain_id)
    if not protected_object:
        return ProtectedObjectTime(latest_time=None, earliest_time=None, next_time=None)
    next_time = get_next_time(db, resource_id)
    return ProtectedObjectTime(latest_time=protected_object.latest_time,
                               earliest_time=protected_object.earliest_time,
                               next_time=next_time)


@exter_attack
@api.get(
    "/protected-objects/sla-compliance",
    status_code=200, summary="查询SLA遵从度汇总数量",
    response_model=ProtectedObjectSlaCompliance)
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR})
def query_protection_compliance(db: Session = async_depend(db_config.get_db_session),
                                user_info: dict = async_depend(_resolve_user_info)):
    return ProtectedObjectService.query_sla_compliance(db, user_info)


@exter_attack
@api.get("/internal/protected-objects/compliance/statistic",
         status_code=200, summary="保护对象遵从度统计【内部接口】",
         response_model=ProtectedObjectSlaCompliance)
def query_protection_compliance(data_base: Session = async_depend(db_config.get_db_session)):
    return ProtectedObjectService.query_sla_compliance(data_base, None)


@exter_attack
@api.get("/internal/protected-objects/resource/count-list", status_code=200,
         summary="根据用户id查询sla关联资源数量【内部接口】",
         response_model=List[SlaResourceQuantityRelationship])
def count_protection_object_list(session: Session = async_depend(db_config.get_db_session),
                                 domain_id: str = Query(None, description="域ID")):
    return ProtectedObjectService.query_projected_object_by_user_id(session=session, domain_id=domain_id)


@exter_attack
@api.put("/internal/protected-objects/backup-esn", status_code=200,
         summary="修改受保护对象的扩展字段")
def modify_protected_obj_backup_esn(database: Session = async_depend(db_config.get_db_session),
                                    submit_req: ModifyProtectObjExtInfoReq = Body(...,
                                                                                  description="设置下次备份参数")):
    protect_obj = ProtectedObjectService.query_obj(session=database, resource_id=submit_req.resource_id)
    if protect_obj is None:
        log.info(f'no protect object exist, resource_id: {submit_req.resource_id}')
        return
    ext_parameters, submit_ext_parameters = integrate_extend_field(protect_obj, submit_req)

    update_conditions = {"ext_parameters": json.dumps(ext_parameters)}
    curd.projected_object.update_by_params(database, submit_req.resource_id, update_conditions)
    log.debug(f'backup esn updated, resource id is {submit_req.resource_id}')


def integrate_extend_field(protect_obj, submit_req):
    submit_ext_parameters = submit_req.ext_parameters
    protect_obj_ext_params = protect_obj.ext_parameters
    if isinstance(protect_obj_ext_params, dict):
        ext_parameters = protect_obj_ext_params
    else:
        ext_parameters = json.loads(protect_obj.ext_parameters)
    if submit_ext_parameters.first_backup_esn:
        ext_parameters['first_backup_esn'] = submit_ext_parameters.first_backup_esn
    if submit_ext_parameters.last_backup_esn:
        ext_parameters['last_backup_esn'] = submit_ext_parameters.last_backup_esn
    if submit_ext_parameters.priority_backup_esn:
        ext_parameters['priority_backup_esn'] = submit_ext_parameters.priority_backup_esn
    if submit_ext_parameters.failed_node_esn:
        ext_parameters['failed_node_esn'] = submit_ext_parameters.failed_node_esn
    if submit_ext_parameters.first_backup_target:
        ext_parameters['first_backup_target'] = submit_ext_parameters.first_backup_target
    if submit_ext_parameters.last_backup_target:
        ext_parameters['last_backup_target'] = submit_ext_parameters.last_backup_target
    if submit_ext_parameters.priority_backup_target:
        ext_parameters['priority_backup_target'] = submit_ext_parameters.priority_backup_target
    if submit_req.delete_keys:
        for key in submit_req.delete_keys:
            try:
                ext_parameters.pop(key)
            except KeyError:
                pass
    return ext_parameters, submit_ext_parameters


@exter_attack
@api.put(
    "/internal/protected-objects/self-learning-progress", status_code=200,
    summary="更新自学习进度【内部接口】"
)
def internal_update_self_learning_progress(
        db: Session = async_depend(db_config.get_db_session),
        update_self_learning_req: UpdateSelfLearningProgressReq = Body(..., description="更新自学习进度的请求"),
):
    protected_object = curd.projected_object.query_one_by_resource_id(db, update_self_learning_req.resource_id)
    if protected_object is None or protected_object.ext_parameters is None:
        log.warn(f"self learning for resource {update_self_learning_req.resource_id} protected object is None!")
        return
    ext_parameters = json.loads(protected_object.ext_parameters)
    ext_parameters.update({
        "progress": update_self_learning_req.progress
    })
    curd.projected_object.update_ext_parameters_by_resource_id(db=db, resource_id=update_self_learning_req.resource_id,
                                                               ext_parameters=json.dumps(ext_parameters))
    log.info(f"update self learning progress resource: {update_self_learning_req.resource_id} " +
             f"ext_parameters: {ext_parameters}")
