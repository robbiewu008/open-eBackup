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
from datetime import datetime
from typing import Dict, List

from sqlalchemy.orm import Query, Session

from app.backup.client.protection_client import ProtectionClient
from app.backup.client.rbac_client import RBACClient
from app.backup.common.config import db_config
from app.backup.models.qos_table import QosTable
from app.backup.schemas.qos import QosReq
from app.common import logger
from app.common.enums.rbac_enum import ResourceSetTypeEnum, ResourceSetScopeModuleEnum
from app.common.exception.protection_error_codes import ProtectionErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exception.user_error_codes import UserErrorCodes
from app.common.extension import to_orm
from app.common.security.jwt_utils import get_user_info_from_token
from app.resource.models.rbac_models import ResourceSetResourceObjectTable, DomainResourceObjectTable
from app.resource.service.host.host_service import get_domain_id_list
from app.common.schemas.resource_set_relation_schemas import ResourceSetRelationInfo

MAX_QOS_COUNT = 64

log = logger.get_logger(__name__)


def qos_authenticate(token):
    user_info = get_user_info_from_token(token)
    domain_id = user_info.get("domain-id")

    def initiator(query: Query) -> Query:
        with db_config.get_session() as session:
            sub_query = session.query(DomainResourceObjectTable.resource_object_id).filter(
                DomainResourceObjectTable.domain_id == domain_id).filter(
                DomainResourceObjectTable.type == ResourceSetTypeEnum.QOS.value).subquery()
            query = query.filter(QosTable.uuid.in_(sub_query))
        return query

    return initiator


def create_qos(db, qos: QosReq, domain_id: str):
    '''
    创建限速策略 规格64，超过64条，不支持新增
    :param db: 数据库
    :param qos: 创建的
    :param user_id:用户id
    :return:无返回值
    '''
    param_check(db, qos, False, None)
    qos = to_orm(qos, QosTable)
    init_qos(qos)
    db.add(qos)
    resource_set_relation_info = ResourceSetRelationInfo(resource_object_id=qos.uuid,
                                                         resource_set_type=ResourceSetTypeEnum.QOS.value,
                                                         scope_module=ResourceSetScopeModuleEnum.QOS.value,
                                                         domain_id_list=get_domain_id_list(domain_id))
    RBACClient.add_resource_set_relation(resource_set_relation_info)


def init_qos(qos: QosTable):
    qos.is_used = False
    qos.created_time = str(datetime.now())
    qos.uuid = str(uuid.uuid4())


def param_check(db, qos: QosReq, is_registered: bool, qos_id: str):
    '''
    qos参数校验

    :param db: 数据库
    :param qos: qos信息
    :param is_registered: 该qos是否是修改，false新增，true修改
    :param qos_id: id
    :return: none
    '''
    if is_registered and db.query(QosTable).filter(QosTable.name == qos.name).filter(
            QosTable.uuid != qos_id).count() > 0:
        raise EmeiStorBizException(ProtectionErrorCodes.QOS_NAME_REPEAT, qos.name, message=f"name is repeat")
    if is_registered is False and db.query(QosTable).filter(QosTable.name == qos.name).count() > 0:
        raise EmeiStorBizException(ProtectionErrorCodes.QOS_NAME_REPEAT, qos.name, message=f"name is repeat")
    if is_registered is False and db.query(QosTable).count() >= MAX_QOS_COUNT:
        raise EmeiStorBizException(ProtectionErrorCodes.QOS_MAX_COUNT, message=f"qos count is max")


def delete_qos(db, ids: List[str]):
    '''
    批量删除限速策略

    :param db:数据库
    :param ids:批量删除的id
    :return:无返回值
    '''
    qos_objects = db.query(QosTable).filter(QosTable.uuid.in_(ids)).all()
    for qos in qos_objects:
        if not can_delete_qos(db, qos):
            return
    for qos in qos_objects:
        db.delete(qos)
        log.info(f"Delete qos success, qos_id: {qos.uuid}, qos_name: {qos.name}")
        RBACClient.delete_resource_set_relation([qos.uuid], ResourceSetTypeEnum.QOS.value)


def can_delete_qos(db: Session, qos: QosTable):
    """
    检查是否能删除限速策略，如果绑定sla不支持删除

    :param db: DataBase连接
    :param qos: 数据库
    :return: 支持返回True,不支持返回False
    """
    sla_list = ProtectionClient.query_sla_by_ext_parameters("qos_id", qos.uuid, False)
    if sla_list:
        name = sla_list[0].get("name")
        raise EmeiStorBizException(ProtectionErrorCodes.QOS_OPERATION_FAIL, name)
    return True


def update_qos(db, qos_id, qos: QosReq):
    '''
    修改限速策略
    :param db:数据库
    :param qos_id:qos Id
    :param qos:更新的信息
    :return:无返回值
    '''
    param_check(db, qos, True, qos_id)
    data = {QosTable.description: qos.description}
    data[QosTable.speed_limit] = qos.speed_limit
    data[QosTable.name] = qos.name
    db.query(QosTable).filter(QosTable.uuid == qos_id).update(data)


def query_qos_by_id(db, qos_id: str):
    return db.query(QosTable).filter(QosTable.uuid == qos_id).first()


def qos_data_condition_filter(condition: Dict[str, any]):
    name = condition.get("name")

    def initiator(query: Query) -> Query:
        if name:
            query = query.filter(QosTable.name == name)
        return query

    return initiator


def verify_qos_ownership(db, user_id, qos_uuid_list):
    '''
    分域资源操作权限校验

    :param db: qos数据库
    :param user_id: 用户id
    :param qos_uuid_list: qos资源id
    :return: 如果没有权限，则抛出异常，如果有权限，正常响应，没有返回值
    '''
    if not qos_uuid_list:
        return
    if not user_id:
        raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)

    count = db.query(QosTable.uuid).filter(QosTable.uuid.in_(qos_uuid_list)).filter(
        QosTable.user_id == user_id).count()
    if count != len(qos_uuid_list):
        raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)


def revoke_qos_user_id(user_id, db):
    db.query(QosTable).filter(
        QosTable.user_id == user_id
    ).update({QosTable.user_id: None})


def condition_filter(condition: Dict[str, any]):
    if 'userInfoForLabel' in condition:
        del condition['userInfoForLabel']

    def initiator(query: Query, session: Session) -> Query:
        if condition:
            resource_set_id = condition.get("resource_set_id")
            if resource_set_id:
                sub_query = session.query(ResourceSetResourceObjectTable.resource_object_id).filter(
                    ResourceSetResourceObjectTable.resource_set_id == resource_set_id).subquery()
                query = query.filter(QosTable.uuid.in_(sub_query))
        return query

    return initiator


def query_qos_list(db):
    return db.query(QosTable).all()


def get_all_count(db):
    return db.query(QosTable.uuid).count()