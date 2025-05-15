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

from fastapi import Body, Header, Query
from sqlalchemy.orm import Session

from app.common.auth.check_ath import CheckAuthModel
from app.common.enums.rbac_enum import ResourceSetTypeEnum, OperationTypeEnum, AuthOperationEnum
from app.common.exter_attack import exter_attack
from app.common.security.jwt_utils import get_user_info_from_token
from app.copy_catalog.common.constant import CopyGeneratedType
from app.copy_catalog.util.copy_auth_verify_util import check_resource_related_copy_auth
from app.protection.object.common import db_config
from app.protection.object.common.constants import CommonOperationID
from app.protection.object.schemas.protected_copy_object import ProtectedCopyObjectId, ProtectedCopyObjectUpdate, \
    ProtectedCopyBatchOperationReq, ManualReplicationReq, SlaResourceQuantityRelationship, ProtectedCopyBaseExtParam
from app.protection.object.service.projected_copy_object_service import ProtectedCopyObjectService
from app.common import logger
from app.common.log.operation_log import operation_log
from app.common.security.right_control import right_control
from app.common.security.role_dict import RoleEnum
from app.common.concurrency import async_route, async_depend
from app.resource.service.common.user_domain_service import get_domain_id_by_user_id

api = async_route()
log = logger.get_logger(__name__)


@exter_attack
@api.post("/protected-copy-objects", status_code=200, summary="创建副本保护对象",
          description="创建副本保护对象，绑定SLA",
          response_model=ProtectedCopyObjectId)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    resources=["sla:create_req.sla_id", "protected_copy:create_req.resource_id"]
)
@operation_log(
    name=CommonOperationID.CREATED_PROTECTION,
    target="@Protection",
    detail=('create_req.resource_id!copyResource.resourceName', '@protection_without_backup_label')
)
def create(
        db: Session = async_depend(db_config.get_db_session),
        token: str = Header(..., alias="X-Auth-Token", title="X-Auth-Token", description="访问令牌"),
        create_req: ProtectedCopyObjectUpdate = Body(..., description="创建副本保护对象请求")):
    user_info = get_user_info_from_token(token)
    domain_id = get_domain_id_by_user_id(user_info.get("user-id"))
    check_resource_related_copy_auth(create_req.resource_id, [AuthOperationEnum.REPLICATION], domain_id,
                                     CopyGeneratedType.COPY_GENERATED_BY_REPLICATED_LIST)
    obj_id = ProtectedCopyObjectService.create_protected_object(
        session=db, create_req=create_req)
    return ProtectedCopyObjectId(uuid=obj_id)


@exter_attack
@api.put("/protected-copy-objects", status_code=200, summary="修改副本保护对象", description="修改副本保护对象，变更SLA")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    resources=["sla:update_req.sla_id", "protected_copy:update_req.resource_id"]
)
@operation_log(
    name=CommonOperationID.MODIFY_PROTECTION,
    target="@Protection",
    detail='update_req.resource_id!copyResource.resourceName'
)
def modify(
        db: Session = async_depend(db_config.get_db_session),
        token: str = Header(..., alias="X-Auth-Token", title="X-Auth-Token", description="访问令牌"),
        update_req: ProtectedCopyObjectUpdate = Body(..., description="修改副本保护对象请求")):
    user_info = get_user_info_from_token(token)
    domain_id = get_domain_id_by_user_id(user_info.get("user-id"))
    check_resource_related_copy_auth(update_req.resource_id, [AuthOperationEnum.REPLICATION], domain_id,
                                     CopyGeneratedType.COPY_GENERATED_BY_REPLICATED_LIST)
    obj_id = ProtectedCopyObjectService.modify_protection(
        session=db, update_req=update_req)
    return ProtectedCopyObjectId(uuid=obj_id)


@exter_attack
@api.delete("/protected-copy-objects", status_code=200, summary="批量移除副本保护对象",
            description="批量移除副本保护对象，解绑SLA")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    resources="protected_copy:req.resource_ids"
)
@operation_log(
    name=CommonOperationID.REMOVE_PROTECTION,
    target="@Protection",
    detail='req.resource_ids!copyResource.resourceName!!string'
)
def delete(
        db: Session = async_depend(db_config.get_db_session),
        token: str = Header(..., alias="X-Auth-Token", title="X-Auth-Token", description="访问令牌"),
        req: ProtectedCopyBatchOperationReq = Body(..., description="批量移除副本保护对象请求")):
    user_info = get_user_info_from_token(token)
    domain_id = get_domain_id_by_user_id(user_info.get("user-id"))
    if not req.resource_ids:
        return
    for resource_id in req.resource_ids:
        check_resource_related_copy_auth(resource_id, [AuthOperationEnum.REPLICATION], domain_id,
                                         CopyGeneratedType.COPY_GENERATED_BY_REPLICATED_LIST)
    ProtectedCopyObjectService.batch_remove_protection(session=db, resource_ids=req.resource_ids)


@exter_attack
@api.put("/protected-copy-objects/status/action/activate", status_code=200, summary="批量激活保护副本")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    resources="protected_copy:req.resource_ids"
)
@operation_log(
    name=CommonOperationID.ACTIVATE_PROTECTION,
    target="@Protection",
    detail='req.resource_ids!copyResource.resourceName!!string'
)
def active(
        db: Session = async_depend(db_config.get_db_session),
        token: str = Header(..., alias="X-Auth-Token", title="X-Auth-Token", description="访问令牌"),
        req: ProtectedCopyBatchOperationReq = Body(..., description="批量激活保护副本操作请求")):
    user_info = get_user_info_from_token(token)
    domain_id = get_domain_id_by_user_id(user_info.get("user-id"))
    if not req.resource_ids:
        return
    for resource_id in req.resource_ids:
        check_resource_related_copy_auth(resource_id, [AuthOperationEnum.REPLICATION], domain_id,
                                         CopyGeneratedType.COPY_GENERATED_BY_REPLICATED_LIST)
    ProtectedCopyObjectService.batch_activated(session=db, resource_ids=req.resource_ids)


@exter_attack
@api.put("/protected-copy-objects/status/action/deactivate", status_code=200, summary="批量取消激活保护副本")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    resources="protected_copy:req.resource_id"
)
@operation_log(
    name=CommonOperationID.DEACTIVATE_PROTECTION,
    target="@Protection",
    detail='req.resource_ids!copyResource.resourceName!!string'
)
def deactivate(
        db_session: Session = async_depend(db_config.get_db_session),
        token: str = Header(..., alias="X-Auth-Token", title="X-Auth-Token", description="访问令牌"),
        req: ProtectedCopyBatchOperationReq = Body(..., description="批量取消激活保护副本操作请求")):
    user_info = get_user_info_from_token(token)
    domain_id = get_domain_id_by_user_id(user_info.get("user-id"))
    if not req.resource_ids:
        return
    for resource_id in req.resource_ids:
        check_resource_related_copy_auth(resource_id, [AuthOperationEnum.REPLICATION], domain_id,
                                         CopyGeneratedType.COPY_GENERATED_BY_REPLICATED_LIST)
    ProtectedCopyObjectService.batch_deactivate(session=db_session, resource_ids=req.resource_ids)


@exter_attack
@api.put("/internal/protected-copy-objects/action/sync-sla", status_code=200,
         summary="副本保护对象同步SLA信息变更【内部接口】")
def sync_replica_sla_change(db: Session = async_depend(db_config.get_db_session),
                            sla_id: str = Body(..., description="SLA的ID", embed=True)):
    ProtectedCopyObjectService.sync_replica_sla_change(session=db, sla_id=sla_id)


def _resolve_user_info(token: str = Header(..., alias="X-Auth-Token", title="X-Auth-Token",
                                           description="访问令牌")) -> dict:
    return get_user_info_from_token(token)


@api.post("/protected-copy-objects/action/replicate", status_code=200, summary="手动复制")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    resources=["resource:manual_replicate_req.resource_id", "protected_copy:manual_replicate_req.resource_id"]
)
@operation_log(
    name=CommonOperationID.PROTECTED_COPY_REPLICATION,
    target="@Replication",
    detail='manual_replicate_req.resource_id!copyResource.resourceName'
)
def manual_replicate(
        user_info: dict = async_depend(_resolve_user_info),
        manual_replicate_req: ManualReplicationReq = Body(..., description="手动复制请求"),
        db: Session = async_depend(db_config.get_db_session)
):
    domain_id = get_domain_id_by_user_id(user_info.get("user-id"))
    check_resource_related_copy_auth(manual_replicate_req.resource_id, [AuthOperationEnum.REPLICATION], domain_id,
                                     CopyGeneratedType.COPY_GENERATED_BY_REPLICATED_LIST)
    ProtectedCopyObjectService.manual_replicate(user_id=user_info.get("user-id"),
                                                manual_replicate_req=manual_replicate_req,
                                                session=db)


@exter_attack
@api.get("/internal/protected-copy-objects/action/count", status_code=200,
         summary="根据SLA的ID查询关联副本数量【内部接口】")
def count_protection_object(session: Session = async_depend(db_config.get_db_session),
                            sla_id: str = Query(..., description="SLA的ID")):
    return ProtectedCopyObjectService.count_by_sla_id(session=session, sla_id=str(sla_id))


@exter_attack
@api.get("/internal/protected-copy-objects/action/verify", status_code=200,
         summary="根据资源ID列表和用户ID校验复制副本资源所属用户是否正确【内部接口】")
def verify_protect_copy_objects_ownership(user_id: str = Query(..., description="用户ID"),
                                          resource_uuid_list: List[str] = Query(..., description="资源ID列表")):
    return ProtectedCopyObjectService.verify_protect_copy_objects_ownership(user_id, resource_uuid_list)


@exter_attack
@api.get("/internal/protected-copy-objects/action/count-list", status_code=200,
         summary="查询所有sla关联副本数量的关系【内部接口】",
         response_model=List[SlaResourceQuantityRelationship])
def count_protection_object_list(session: Session = async_depend(db_config.get_db_session)):
    return ProtectedCopyObjectService.query_copy_projected_object(session=session)
