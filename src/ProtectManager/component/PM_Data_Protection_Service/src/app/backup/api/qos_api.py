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
from typing import List, Optional

from fastapi import Body, Header, Path, Query
from sqlalchemy.orm import Session

from app.backup.common.config import db_config
from app.backup.common.config.base import database
from app.backup.models.qos_table import QosTable
from app.backup.schemas.qos import QosReq, QosRes
from app.backup.service import qos_service
from app.backup.service.qos_service import qos_authenticate
from app.common import logger
from app.common.auth.check_ath import CheckAuthModel
from app.common.concurrency import async_route, async_depend
from app.common.enums.rbac_enum import ResourceSetTypeEnum, OperationTypeEnum, AuthOperationEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.extension import define_page_query_api_for_model
from app.common.exter_attack import exter_attack
from app.common.log.common_operation_code import OperationAlarmCode
from app.common.log.operation_log import operation_log
from app.common.security.jwt_utils import get_user_domain_id
from app.common.security.right_control import right_control
from app.common.security.role_dict import RoleEnum

qos_api = async_route()
logger = logger.get_logger(__name__)

QOS_TAG = "Qos"

define_page_query_api_for_model(
    qos_api,
    database,
    QosTable,
    initiator=qos_service.condition_filter,
    extra_conditions=["resource_set_id"],
    path="/qos",
    tags=[QOS_TAG],
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR},
    authenticate=qos_authenticate,
    summary="分页查询Qos策略列表")

define_page_query_api_for_model(
    qos_api,
    database,
    QosTable,
    path="/internal/qos",
    tags=[QOS_TAG],
    summary="分页查询Qos策略列表"
)


@exter_attack
@qos_api.post("/qos", status_code=200, summary="创建Qos策略", tags=[QOS_TAG])
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
               check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.QOS, operation=OperationTypeEnum.CREATE,
                                         auth_operation_list={AuthOperationEnum.SPEED_LIMIT_STRATEGY}))
@operation_log(
    name=OperationAlarmCode.CREATE_QOS,
    target="@Protection",
    detail=("qos_req.name")
)
def create_qos(qos_req: QosReq = Body(..., description="创建Qos策略请求"),
               token: str = Header(..., alias="X-Auth-Token", description="授权token"),
               db: Session = async_depend(db_config.get_db_session)):
    domain_id = get_user_domain_id(token)
    qos_service.create_qos(db, qos_req, domain_id)


@exter_attack
@qos_api.delete("/qos", status_code=200, summary="批量删除指定Qos策略", tags=[QOS_TAG])
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="qos:qos_ids",
               check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.QOS, operation=OperationTypeEnum.DELETE,
                                         auth_operation_list={AuthOperationEnum.SPEED_LIMIT_STRATEGY},
                                         target="qos_ids")
               )
@operation_log(
    name=OperationAlarmCode.DELETE_QOS,
    target="@Protection",
    detail=("qos_ids!qos.name!!string")
)
def delete_qos(qos_ids: List[str] = Body(..., description="Qos ids"),
               db: Session = async_depend(db_config.get_db_session)):
    '''
    删除qos
    :param qos_ids: qos 最大规格为64
    :param db: db
    :return: none
    '''
    max_items = 64
    max_len = 64
    if qos_ids is None or len(qos_ids) > max_items or len(qos_ids) == 0:
        raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM,
                                   message="uuid len is invalid")
    for qos_id in qos_ids:
        if len(qos_id) > max_len:
            raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM,
                                       message="uuid is invalid")
    qos_service.delete_qos(db, qos_ids)


@exter_attack
@qos_api.put("/qos/{qos_id}", status_code=200, summary="更新指定Qos策略", tags=[QOS_TAG])
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="qos:qos_id",
               check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.QOS, operation=OperationTypeEnum.MODIFY,
                                         auth_operation_list={AuthOperationEnum.SPEED_LIMIT_STRATEGY},
                                         target="qos_id")
               )
@operation_log(
    name=OperationAlarmCode.UPDATE_QOS,
    target="@Protection",
    detail=("qos_id!qos.name")
)
def update_qos(qos_id: str = Path(..., description="Qos的id", max_length=64),
               qos_req: QosReq = Body(..., description="Qos信息"),
               db: Session = async_depend(db_config.get_db_session)):
    qos_service.update_qos(db, qos_id, qos_req)


@exter_attack
@qos_api.get("/qos/{qos_id}", status_code=200, summary="查询指定Qos策略", response_model=Optional[QosRes],
             response_description="qos详细信息", tags=[QOS_TAG])
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR}, resources="qos:qos_id",
               check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.QOS, operation=OperationTypeEnum.QUERY,
                                         auth_operation_list={AuthOperationEnum.SPEED_LIMIT_STRATEGY},
                                         target="qos_id")
               )
def get_qos(qos_id: str = Path(..., description="Qos的id", max_length=64),
            db: Session = async_depend(db_config.get_db_session)):
    return qos_service.query_qos_by_id(db, qos_id)


@exter_attack
@qos_api.get("/internal/qos/{qos_id}", status_code=200, summary="Get Qos", response_model=Optional[QosRes],
             response_description="qos详细信息", tags=[QOS_TAG])
def internal_get_qos(qos_id: str = Path(..., description="Qos的id"),
                     db: Session = async_depend(db_config.get_db_session)):
    return qos_service.query_qos_by_id(db, qos_id)


@exter_attack
@qos_api.get("/internal/qos/list", status_code=200, summary="Get Qos", response_model=Optional[QosRes],
             response_description="qos列表", tags=[QOS_TAG])
def internal_get_qos_list(db: Session = async_depend(db_config.get_db_session)):
    return qos_service.query_qos_list(db)


@exter_attack
@qos_api.get("/internal/qos/action/verify", status_code=200, tags=[QOS_TAG])
def internal_verify_qos_ownership(user_id: str = Query(..., description="用户ID"),
                                  qos_uuid_list: List[str] = Query(..., description="Qos id列表"),
                                  db: Session = async_depend(db_config.get_db_session)):
    qos_service.verify_qos_ownership(db, user_id, qos_uuid_list)


@exter_attack
@qos_api.put("/internal/qos/action/revoke/{user_id}", status_code=200, tags=[QOS_TAG])
def revoke_qos_user_id(user_id: str = Path(..., description="用户ID"),
                       db: Session = async_depend(db_config.get_db_session)):
    qos_service.revoke_qos_user_id(user_id, db)


@exter_attack
@qos_api.get("/internal/all-count", status_code=200, tags=[QOS_TAG])
def get_all_count(db: Session = async_depend(db_config.get_db_session)):
    return qos_service.get_all_count(db)