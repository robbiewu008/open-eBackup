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
from collections import Counter
from typing import Dict, List

from sqlalchemy import false, and_, or_, func
from sqlalchemy.orm import Query, Session, aliased

from app.base.clean_except import clean_except
from app.base.db_base import database
from app.base.resource_consts import ResourceConstant
from app.common import logger
from app.common.clients.system_base_client import SystemBaseClient, LOGGER
from app.common.enums.license_enum import FunctionEnum
from app.common.enums.rbac_enum import ResourceSetTypeEnum
from app.common.enums.resource_enum import LinkStatusEnum, ResourceSubTypeEnum
from app.common.enums.resource_enum import ResourceTypeEnum, ProtectionStatusEnum
from app.common.event_messages.Discovery.discovery_rest import ResourceStatus, AuthType
from app.common.event_messages.event import CommonEvent
from app.common.events import producer
from app.common.events.topics import RESOURCE_DELETED_TOPIC
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exception.user_error_codes import UserErrorCodes
from app.common.extension import get_all_model_types, get_mapper_arg, paginator
from app.common.extension import get_valid_paginate_config_for_model, get_relation_schema
from app.common.license import validate_license_by_resource_type
from app.common.security.jwt_utils import get_user_info_from_token, check_user_is_admin
from app.common.security.role_dict import RoleEnum
from app.copy_catalog.models.tables_and_sessions import ClusterMemberTable
from app.protection.object.common import db_config
from app.protection.object.models.projected_object import ProtectedObject
from app.resource.common.constants import ResourceConstants
from app.resource.models.database_models import DatabaseTable
from app.resource.models.import_resource_models import ImportResourceTable
from app.resource.models.rbac_models import ResourceSetResourceObjectTable, DomainResourceObjectTable, UserTable
from app.resource.models.resource_models import ResourceTable, EnvironmentTable, ResExtendInfoTable, TClusterTarget, \
    TClusterLocal, TDistributionStorage, AgentTable
from app.resource.models.resource_group_models import ResourceGroup, ResourceGroupMember
from app.resource.rpc import hw_agent_rpc
from app.resource.schemas.resource import ResourceProtectionCount
from app.resource.schemas.resource import ResourceProtectionSummary
from app.resource.service.common.ownership_dispatcher import OwnershipDispatcher
from app.resource.models.label_models import LabelTable, LabelResourceTable
from app.resource.service.common.user_domain_service import get_domain_id_by_user_id

log = logger.get_logger(__name__)


def resource_authenticate(token):
    user_info = get_user_info_from_token(token)
    domain_id = user_info.get("domain-id")

    def initiator(query: Query, session: Session) -> Query:
        sub_query = session.query(DomainResourceObjectTable.resource_object_id).filter(
            DomainResourceObjectTable.domain_id == domain_id).subquery()
        query = query.filter(ResourceTable.uuid.in_(sub_query))
        return query

    return initiator


def resource_and_license_authenticate(token):
    validate_license_by_resource_type(FunctionEnum.DATA_MANANGE_GLOBALE_SEARCH, ResourceSubTypeEnum.Fileset)
    return resource_authenticate(token)


def apply_label_name_filters(query, condition, session: Session, user_info):
    label_name = condition['labelName']
    if check_user_is_admin(user_info):
        label_query = session.query(LabelResourceTable.resource_object_id) \
            .join(LabelTable, LabelResourceTable.label_id == LabelTable.uuid) \
            .filter(LabelTable.name.ilike(f"%{label_name}%")).subquery()
        return query.filter(ResourceTable.uuid.in_(label_query))
    label_query = session.query(LabelResourceTable.resource_object_id) \
        .join(LabelTable, LabelResourceTable.label_id == LabelTable.uuid) \
        .filter(
        LabelTable.name.ilike(f"%{label_name}%"),
        LabelTable.builder_name == user_info.get('user-name')
    ).subquery()
    return query.filter(ResourceTable.uuid.in_(label_query))


def apply_label_list_filters(query, condition, session: Session):
    label_list = condition['labelList']
    if isinstance(label_list, list) and all(isinstance(label, str) for label in label_list):
        resource_ids = session.query(LabelResourceTable.resource_object_id) \
            .filter(LabelResourceTable.label_id.in_(label_list)) \
            .group_by(LabelResourceTable.resource_object_id) \
            .having(func.count(LabelResourceTable.label_id) == len(label_list)) \
            .subquery()
        return query.filter(ResourceTable.uuid.in_(resource_ids))
    return query


def resource_condition_filter(condition: Dict[str, any]):
    default_condition_filter = None
    if not condition:
        return default_condition_filter
    if 'userInfoForLabel' in condition:
        user_info = condition['userInfoForLabel']
        del condition['userInfoForLabel']
    resource_name = condition.get("resource_name")
    user_id = condition.get("user_id")
    resource_set_id = condition.get("resource_set_id")

    def initiator(query: Query, session: Session) -> Query:
        if resource_name:
            query = query.filter(ResourceTable.name == resource_name)
        if user_id:
            subquery = session.query(EnvironmentTable.uuid).filter(
                EnvironmentTable.user_id == user_id).subquery()
            query = query.filter(ResourceTable.root_uuid.in_(subquery))
        if resource_set_id:
            sub_query = session.query(ResourceSetResourceObjectTable.resource_object_id).filter(
                ResourceSetResourceObjectTable.resource_set_id == resource_set_id).subquery()
            query = query.filter(ResourceTable.uuid.in_(sub_query))
        if 'labelName' in condition:
            query = apply_label_name_filters(query, condition, session, user_info)

        if 'labelList' in condition:
            query = apply_label_list_filters(query, condition, session)
        return query

    return initiator


def global_search_condition_filter(condition: Dict[str, any]):
    if 'userInfoForLabel' in condition:
        user_info = condition['userInfoForLabel']
        del condition['userInfoForLabel']

    def initiator(query: Query, session: Session) -> Query:
        query = query.filter(ResourceTable.sub_type != ResourceSubTypeEnum.DWSDateBase.value)
        query = query.filter(ResourceTable.type != ResourceTypeEnum.PLUGIN.value)
        if condition:
            sub_query = session.query(DatabaseTable.uuid).filter(DatabaseTable.valid == false()).subquery()
            # 查询出ResExtendInfoTable里面，isTopInstance为0的数据的资源id，作为resource查询的过滤条件
            # 这些非顶级资源的子资源，不展示在全局查询中
            extend_query = session.query(ResExtendInfoTable.resource_id) \
                .filter(ResExtendInfoTable.key == "isTopInstance", ResExtendInfoTable.value == "0").subquery()
            if 'labelName' in condition:
                query = apply_label_name_filters(query, condition, session, user_info)

            if 'labelList' in condition:
                query = apply_label_list_filters(query, condition, session)

            query = query.filter(ResourceTable.uuid.notin_(sub_query)).filter(ResourceTable.uuid.notin_(extend_query))
        return query

    return initiator


def vmware_search_condition_filter(condition: Dict[str, any]):
    def initiator(query: Query, session: Session) -> Query:
        if condition:
            if 'labelName' in condition:
                label_name = condition['labelName']
                # 获取与指定 labelName 相关的资源 ID 列表
                label_query = session.query(LabelResourceTable.resource_object_id) \
                    .join(LabelTable, LabelResourceTable.label_id == LabelTable.uuid) \
                    .filter(LabelTable.name.ilike(f"%{label_name}%")).subquery()
                query = query.filter(EnvironmentTable.uuid.in_(label_query))

            if 'labelList' in condition:
                query = apply_label_list_filters(query, condition, session)
        return query

    return initiator


def query_resource_by_id(resource_id: str):
    default_query_res = None
    resource_types = get_all_model_types(ResourceTable) + [ResourceTable]
    with database.session() as session:
        count = session.query(ResourceTable.discriminator).filter(
            ResourceTable.uuid == resource_id).count()
        if count == 0:
            return default_query_res
        discriminator = session.query(ResourceTable.discriminator).filter(
            ResourceTable.uuid == resource_id).one()[0]
        matches = list(resource_type for resource_type in resource_types
                       if get_mapper_arg(resource_type, "polymorphic_identity") == discriminator)
        if not matches:
            raise EmeiStorBizException(CommonErrorCodes.SYSTEM_ERROR)
        model_type = matches[0]
        paginate = paginator(database, model_type)
        result = paginate(page=0, size=2, conditions={
            "uuid": resource_id}).one()
        if issubclass(model_type, EnvironmentTable) or issubclass(model_type, DatabaseTable):
            relations = get_valid_paginate_config_for_model(model_type, "__internal_relation__")
        else:
            relations = get_valid_paginate_config_for_model(model_type)
        schema = get_relation_schema(model_type, relations)
        return schema.parse_obj(result)


def query_resource_by_parent_id(parent_id: str) -> List[ResourceTable]:
    with database.session() as session:
        members = session.query(ResourceTable).filter(ResourceTable.root_uuid == parent_id)
        return list(members)


def query_resource_by_parent_id_and_database(parent_id: str, database_name: str) -> List[ResourceTable]:
    with database.session() as session:
        members = session.query(ResourceTable) \
            .join(ResExtendInfoTable, ResExtendInfoTable.resource_id == ResourceTable.uuid) \
            .filter(ResourceTable.root_uuid == parent_id) \
            .filter(ResourceTable.sub_type == ResourceSubTypeEnum.EXCHANGE_MAILBOX) \
            .filter(ResExtendInfoTable.key == 'DatabaseName') \
            .filter(ResExtendInfoTable.value == database_name).all()
    return list(members)


def query_resource_by_child_id(uuid: str, parent_id: str):
    with database.session() as session:
        members = session.query(ResExtendInfoTable).filter(ResExtendInfoTable.resource_id == uuid) \
            .filter(ResExtendInfoTable.key == 'DatabaseName')
        database_name = members[0].value
        resource_members = session.query(ResourceTable).filter(ResourceTable.root_uuid == parent_id) \
            .filter(ResourceTable.sub_type == ResourceSubTypeEnum.EXCHANGE_DATABASE) \
            .filter(ResourceTable.name == database_name)
        return resource_members[0]


def query_resource_group_by_id(resource_group_id: str) -> ResourceGroup:
    with database.session() as session:
        return session.query(ResourceGroup).filter(ResourceGroup.uuid == resource_group_id).first()


def query_resource_group_members_count(source_id_list: List[str]) -> int:
    with database.session() as session:
        count = session.query(ResourceGroupMember).filter(ResourceGroupMember.source_id.in_(source_id_list)).count()
        return count


def query_resource_group_members_by_group_id(resource_group_id: str) -> List[ResourceGroupMember]:
    with database.session() as session:
        members = session.query(ResourceGroupMember).filter(ResourceGroupMember.resource_group_id == resource_group_id)
        return list(members)


def query_resource_group_member_by_resource_id(resource_id: str) -> ResourceGroupMember:
    with database.session() as session:
        return session.query(ResourceGroupMember).filter(ResourceGroupMember.source_id == resource_id).first()


def delete_resource(resource_id: str, resource_type: str = None):
    with database.session() as session:
        query: Query = session.query(ProtectedObject).filter(
            ProtectedObject.resource_id == resource_id)
        query = query if not resource_type else query.filter(
            ProtectedObject.type == resource_type)
        protect_object: ProtectedObject = query.one_or_none()
        if protect_object is not None and protect_object.sla_id is not None:
            raise EmeiStorBizException(CommonErrorCodes.STATUS_ERROR,
                                       message="Cannot delete because it has been associated with an SLA.")
        query = session.query(ResourceTable).filter(
            ResourceTable.uuid == resource_id)
        query = query if not resource_type else query.filter(
            ResourceTable.type == resource_type)
        resource_object = query.all()
        # 正在创建保护对象的资源不能被删除
        for resource in resource_object:
            if resource.protection_status == ProtectionStatusEnum.protecting.value:
                raise EmeiStorBizException(CommonErrorCodes.STATUS_ERROR,
                                           message="Cannot delete because it has being associated with an SLA.")
        count = query.delete()
        if count > 0:
            session.query(ProtectedObject).filter(
                ProtectedObject.resource_id == resource_id).delete()
    comment_event_message(RESOURCE_DELETED_TOPIC, resource_id=resource_id)


def query_resource(query_expression: dict = None):
    with database.session() as session:
        if not query_expression:
            envs = session.query(ResourceTable).all()
        else:
            envs = session.query(ResourceTable).filter_by(
                **query_expression).all()

        return list(item.as_dict() for item in envs)


def get_os_type_resource_protection_info(session, os_types, filters):
    if not os_types:
        return []
    resource_filters = [
        ResourceTable.type == ResourceTypeEnum.Host.value,
        ResourceTable.sub_type.in_(list(ResourceConstant.HOST_SUBTYPE_MAP.values())),
        or_(ResExtendInfoTable.key.is_(None),
            and_(ResExtendInfoTable.key == "scenario", ResExtendInfoTable.value != "1")
            )
    ]
    resource_filters.extend(filters)
    host_sub_query = session.query(ResourceTable.uuid.label('resource_uuid'), ResourceTable.type.label('type'),
                                   ResourceTable.protection_status.label('protection_status')) \
        .outerjoin(ResExtendInfoTable, ResourceTable.uuid == ResExtendInfoTable.resource_id, full=True) \
        .filter(*resource_filters) \
        .distinct(ResourceTable.uuid) \
        .subquery()
    host_resources = session \
        .query(host_sub_query.c.type, EnvironmentTable.os_type, host_sub_query.c.protection_status) \
        .outerjoin(EnvironmentTable, EnvironmentTable.uuid == host_sub_query.c.resource_uuid) \
        .filter(EnvironmentTable.os_type.in_(os_types), EnvironmentTable.is_cluster == false()) \
        .all()
    return host_resources


def get_other_resource_protection_info(session, sub_types, filters):
    if not sub_types:
        return []
    resource_filters = [ResourceTable.sub_type.in_(sub_types)]
    resource_filters.extend(filters)
    other_resources = session \
        .query(ResourceTable.type, ResourceTable.sub_type, ResourceTable.protection_status) \
        .filter(*resource_filters).all()
    return other_resources


def get_top_inst_resource_protection_info(session, sub_types, filters):
    return get_extend_info_filer_protection_info(session, sub_types, filters, "isTopInstance", "1")


def get_ag_id_resource_protection_info(session, sub_types, filters):
    return get_extend_info_filer_protection_info(session, sub_types, filters, "agId", "")


def get_extend_info_filer_protection_info(session, sub_types, filters, extend_info_key, extend_info_value):
    if not sub_types:
        return []
    resource_filters = [ResourceTable.sub_type.in_(sub_types)]
    resource_filters.extend(filters)
    ag_id_resources = session \
        .query(ResourceTable.type, ResourceTable.sub_type, ResourceTable.protection_status) \
        .outerjoin(ResExtendInfoTable, ResourceTable.uuid == ResExtendInfoTable.resource_id) \
        .filter(*resource_filters) \
        .filter(ResExtendInfoTable.key == extend_info_key, ResExtendInfoTable.value == extend_info_value).all()
    return ag_id_resources


def get_redis_resource_protection_info(session, sub_types, filters):
    if not sub_types:
        return []
    resource_filters = [
        ResourceTable.discriminator == "environments",
        ResourceTable.sub_type.in_(sub_types)
    ]
    resource_filters.extend(filters)
    redis_resources = session \
        .query(ResourceTable.type, ResourceTable.sub_type, ResourceTable.protection_status) \
        .filter(*resource_filters).all()
    return redis_resources


def get_clickhouse_resource_protection_info(session, sub_types, filters):
    clickhouse_type_list = [ResourceTypeEnum.TableSet, ResourceTypeEnum.Database]
    if not sub_types:
        return []
    resource_filters = [
        ResourceTable.discriminator == "resources",
        ResourceTable.sub_type.in_(sub_types),
        ResourceTable.type.in_(clickhouse_type_list)
    ]
    resource_filters.extend(filters)
    redis_resources = session \
        .query(ResourceTable.type, ResourceTable.sub_type, ResourceTable.protection_status) \
        .filter(*resource_filters).all()
    return redis_resources


def filter_cannot_protected_resource(resources: List):
    """
    过滤掉不能对其进行保护的资源，如FusionCompute环境资源
    :param resources: 所有资源
    :return: 过滤后的所有能对齐进行保护的资源
    """
    resource_list = []
    for resource in resources:
        # FusionCompute环境(type == Platform && subType == FusionCompute)无法进行保护，需要过滤掉
        if resource[0] == ResourceTypeEnum.Platform and (resource[1] == ResourceSubTypeEnum.FusionCompute
                                                         or resource[1] == ResourceSubTypeEnum.FUSION_ONE_COMPUTE):
            continue
        resource_list.append(resource)
    return resource_list


def convert_resource_to_summary_schema(resources: List):
    valid_resources = filter_cannot_protected_resource(resources)
    sub_type_list = []
    sub_resources_dict = {}
    for res in valid_resources:
        temp_type = res[0]
        temp_sub_type = res[1]
        if temp_sub_type not in sub_type_list:
            sub_type_list.append(temp_sub_type)
        if temp_sub_type not in sub_resources_dict:
            sub_resources_dict[temp_sub_type] = {
                "resource_type": temp_type,
                "is_protected_list": []
            }
        sub_resources_dict.get(temp_sub_type).get('is_protected_list').append(res[2])
    log.debug("Get sub resources success.")
    sub_resources_details = []
    for sub_type in sub_type_list:
        sub_resource_dict = sub_resources_dict.get(sub_type, {})
        resource_type = sub_resource_dict.get('resource_type')
        is_protected_list = sub_resource_dict.get('is_protected_list', [])
        counter_obj = Counter(is_protected_list)
        protected_count = counter_obj.get(ProtectionStatusEnum.protected.value, 0)
        unprotected_count = counter_obj.get(ProtectionStatusEnum.unprotected.value, 0)
        sub_resources_details.append(
            ResourceProtectionCount(resource_sub_type=sub_type,
                                    resource_type=resource_type,
                                    protected_count=protected_count,
                                    unprotected_count=unprotected_count))
    log.debug("Get sub type success.")
    return ResourceProtectionSummary(summary=sub_resources_details)


def check_sub_types(sub_types: List[str]):
    # 当sub_type不为空的情况下，对其参数进行检验
    if sub_types is not None:
        # 判断size是否超过了正常业务数据下的size
        if len(sub_types) > len(ResourceConstant.SUB_TYPE_ALL_LIST):
            raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, message=f"sub type size illegal.")
        # 判断每一个sub_type是否后端能够识别
        for one_sub_type in sub_types:
            if one_sub_type not in ResourceConstant.SUB_TYPE_ALL_LIST:
                raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS,
                                           message=f"sub type(" + one_sub_type + ") illegal.")


def get_resource_group_protection_info(user_info):
    with database.session() as session:
        filters = []
        if user_info:
            sub_query = session.query(DomainResourceObjectTable.resource_object_id).filter(
                DomainResourceObjectTable.domain_id == user_info.get("domain-id")).filter(
                DomainResourceObjectTable.type.in_(
                    [ResourceSetTypeEnum.RESOURCE.value, ResourceSetTypeEnum.RESOURCE_GROUP])).subquery()
            filters.append(ResourceGroup.uuid.in_(sub_query))
    resource_filters = [ResourceGroup.source_sub_type.in_(ResourceConstant.DIRECT_STATS_SUB_TYPE_LIST)]
    resource_filters.extend(filters)
    other_resources = session \
        .query(ResourceGroup.source_type, ResourceGroup.source_sub_type, ResourceGroup.protection_status) \
        .filter(*resource_filters).all()
    return other_resources


def get_protected_rule_resource_groups(env_id: str, sub_type: str):
    with database.session() as session:
        resources = session \
            .query(ResourceGroup) \
            .filter(ResourceGroup.scope_resource_id == env_id) \
            .filter(ResourceGroup.source_sub_type == sub_type) \
            .filter(ResourceGroup.group_type == 'rule') \
            .filter(ResourceGroup.protection_status == ProtectionStatusEnum.protected) \
            .order_by(ResourceGroup.created_time.asc()) \
            .all()
    return resources


def get_resource_group_by_group_id(group_id: str):
    with database.session() as session:
        resource = session \
            .query(ResourceGroup) \
            .filter(ResourceGroup.uuid == group_id) \
            .filter(ResourceGroup.group_type == 'rule') \
            .filter(ResourceGroup.protection_status == ProtectionStatusEnum.protected) \
            .order_by(ResourceGroup.created_time.asc()) \
            .first()
    return resource


def get_resource_group_by_member(source_id: str):
    with database.session() as session:
        group = session \
            .query(ResourceGroup) \
            .join(ResourceGroupMember, ResourceGroup.uuid == ResourceGroupMember.resource_group_id) \
            .filter(ResourceGroupMember.source_id == source_id) \
            .first()
    return group


def query_protected_resource_by_group_id(resource_group_id: str) -> List[ResourceTable]:
    with database.session() as session:
        return session.query(ResourceTable) \
            .join(ProtectedObject, ResourceTable.uuid == ProtectedObject.uuid, isouter=True) \
            .filter(ProtectedObject.resource_group_id == resource_group_id).all()


def query_protected_resource_by_id(resource_id: str) -> ProtectedObject:
    with database.session() as session:
        return session.query(ProtectedObject) \
            .filter(ProtectedObject.uuid == resource_id).first()


def query_group_resources_by_group_id(group_id: str):
    with database.session() as session:
        resources = session.query(ProtectedObject) \
            .filter(ProtectedObject.resource_group_id == group_id) \
            .filter(ProtectedObject.resource_id != ProtectedObject.resource_group_id) \
            .all()
        resource_id_list = [resource.uuid for resource in resources]
    return resource_id_list


def summary_protection_resource(sub_types: List[str], token):
    # 校验sub_types
    check_sub_types(sub_types)
    resources = []
    # Redis资源子类型
    redis_sub_type_list = [ResourceSubTypeEnum.Redis]
    clickhouse_sub_type_list = [ResourceSubTypeEnum.ClickHouse]
    with database.session() as session:
        user_info = get_user_info_from_token(token)
        log.debug("Decode token success.")
        filters = []
        if user_info:
            sub_query = session.query(DomainResourceObjectTable.resource_object_id).filter(
                DomainResourceObjectTable.domain_id == user_info.get("domain-id")).filter(
                DomainResourceObjectTable.type.in_(
                    [ResourceSetTypeEnum.RESOURCE.value, ResourceSetTypeEnum.RESOURCE_GROUP])).subquery()
            filters.append(ResourceTable.uuid.in_(sub_query))
        if sub_types is None:
            resources.extend(get_other_resource_protection_info(
                session, ResourceConstant.DIRECT_STATS_SUB_TYPE_LIST, filters))
            resources.extend(get_resource_group_protection_info(user_info))
            resources.extend(get_top_inst_resource_protection_info(
                session, ResourceConstant.TOP_INST_SUB_TYPE_LIST, filters))
            resources.extend(get_ag_id_resource_protection_info(session, ResourceConstant.AG_ID_SUB_TYPE_LIST, filters))
            resources.extend(get_redis_resource_protection_info(session, redis_sub_type_list, filters))
            resources.extend(get_clickhouse_resource_protection_info(session, clickhouse_sub_type_list, filters))
        else:
            req_other_sub_type_list = list(set(sub_types).intersection(ResourceConstant.DIRECT_STATS_SUB_TYPE_LIST))
            resources.extend(get_other_resource_protection_info(session, req_other_sub_type_list, filters))
            req_top_inst_sub_type_list = list(set(sub_types).intersection(ResourceConstant.TOP_INST_SUB_TYPE_LIST))
            resources.extend(get_top_inst_resource_protection_info(session, req_top_inst_sub_type_list, filters))
            req_ag_id_sub_type_list = list(set(sub_types).intersection(ResourceConstant.AG_ID_SUB_TYPE_LIST))
            resources.extend(get_ag_id_resource_protection_info(session, req_ag_id_sub_type_list, filters))
            req_redis_sub_type_list = list(set(sub_types).intersection(redis_sub_type_list))
            resources.extend(get_redis_resource_protection_info(session, req_redis_sub_type_list, filters))
            req_clickhouse_sub_type_list = list(set(sub_types).intersection(clickhouse_sub_type_list))
            resources.extend(get_clickhouse_resource_protection_info(session, req_clickhouse_sub_type_list, filters))
        log.debug("Get resources success.")
    return convert_resource_to_summary_schema(resources)


@clean_except()
def query_environment(query_expression: dict = None) -> list:
    with database.session() as session:
        if not query_expression:
            envs = session.query(EnvironmentTable).all()
        else:
            envs = session.query(EnvironmentTable).filter_by(
                **query_expression).all()

        return list(item.as_dict() for item in envs)


def verify_resource_ownership(user_id: str, resource_uuid_list: List[str]):
    if not resource_uuid_list:
        return
    if not user_id:
        raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)
    with database.session() as session:
        environment = aliased(EnvironmentTable)
        uuid_list = session.query(ResourceTable.uuid).filter(ResourceTable.uuid.in_(resource_uuid_list)).all()
        uuid_list = list(item[0] for item in uuid_list)
        domain_id = get_domain_id_by_user_id(user_id)
        resource_object_list = session.query(DomainResourceObjectTable.resource_object_id).filter(
            DomainResourceObjectTable.domain_id == domain_id).all()
        resource_object_ids = [item[0] for item in resource_object_list]
        if not resource_object_ids:
            # 当根据user的domain查询资源的时候 因为查询的结果为空 即不包含任何资源 则后面的校验必然失败(除非resource_uuid_list也是空)
            log.error(f"verify resource ownership, get empty resource list from user:{user_id}, domain_id:{domain_id}")
            if not resource_uuid_list:
                raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)
            else:
                return
        results = session.query(ResourceTable.uuid).join(
            environment, environment.uuid == ResourceTable.root_uuid
        ).filter(
            ResourceTable.uuid.in_(uuid_list)
        ).filter(
            or_(environment.user_id == user_id, ResourceTable.uuid.in_(resource_object_ids))
        ).all()
        # 共享agent所有数据保护管理员都有权限
        results = list(item[0] for item in results)
        shared_agents = session.query(AgentTable.uuid).filter(AgentTable.is_shared.is_(True)).all()
        results.extend(list(shared_agent[0] for shared_agent in shared_agents))
        matched = all(uuid in results for uuid in uuid_list)
        if not matched:
            raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)


def authorize_resource(user_id, resource_uuid_list: List[str]):
    # 检验资源列表里面的资源参数
    if resource_uuid_list is None or user_id is None:
        raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, message="params is illegal")
    for resource_uuid in resource_uuid_list:
        if len(resource_uuid) > 512:
            raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, message="params is illegal")
    with database.session() as session:
        user_info = SystemBaseClient.query_user(user_id)
        roles = list(role['roleName'] for role in user_info.get("rolesSet") or [])

        if RoleEnum.ROLE_DP_ADMIN.value not in roles:
            raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)
        # 如果当前资源是共享主机则直接报错
        if session.query(AgentTable).filter(AgentTable.uuid.in_(resource_uuid_list)).filter(
                AgentTable.is_shared.is_(True)).count() > 0:
            raise EmeiStorBizException(UserErrorCodes.NOT_ALLOW_AUTHORIZE_OR_REVOKE)
        # 检查资源是否已被其他资源依赖
        check_resource_is_depended_on(resource_uuid_list, session)
        # 查询资源对象列表 同时检查资源是否已经绑定sla
        authorize_resource_list = query_authorize_resource_list(
            resource_uuid_list, session)
        authorize_resource_list = search_citation_resource_uuid(authorize_resource_list, session, False)
        # 检查是否已经被授权
        check_all_resources_already_be_authorized(authorize_resource_list, user_id, session)
        # 其它类型资源授权不设置共享主机的授权用户
        session.query(ResourceTable).filter(ResourceTable.uuid.in_(authorize_resource_list)).update(
            {ResourceTable.user_id: user_id,
             ResourceTable.authorized_user: user_info.get("userName")},
            synchronize_session=False)


def filter_shared_agent(authorize_resource_list, session):
    shared_authorize_resource_list = session.query(AgentTable.uuid).filter(
        AgentTable.is_shared.is_(True)).all()
    shared_authorize_resource_list = list(set(i[0] for i in shared_authorize_resource_list))
    unshared_authorize_resource_list = []
    for authorize_resource_uuid in authorize_resource_list:
        if authorize_resource_uuid not in shared_authorize_resource_list:
            unshared_authorize_resource_list.append(authorize_resource_uuid)
    return unshared_authorize_resource_list


def query_authorize_resource_list(resource_uuid_list, session):
    resource_list = session.query(ResourceTable).filter(ResourceTable.uuid.in_(resource_uuid_list)).all()
    authorized_resource_list = []
    authorized_resource_list.extend(resource_uuid_list)
    dispatcher = OwnershipDispatcher(resource_list[0].sub_type)
    for resource in resource_list:
        leaf = dispatcher.query_subset_id_list(resource, session)
        authorized_resource_list.extend(leaf)
    return authorized_resource_list


def search_citation_resource_uuid(resource_uuid_list: List[str], session, is_revoked: bool):
    condition = {
        ResExtendInfoTable.key.like(ResourceConstants.CITATION + ResourceConstants.CITATION_SEPERATOR + '%'),
        ResExtendInfoTable.key.notlike(
            ResourceConstants.CITATION + ResourceConstants.CITATION_SEPERATOR + ResourceConstants.CHILDREN + '%'),
        ResExtendInfoTable.value.in_(resource_uuid_list)
    }
    dependency_resources_uuids = session.query(ResExtendInfoTable.resource_id).filter(*condition).all()
    dependency_resources_uuids = list(set(i[0] for i in dependency_resources_uuids))

    if is_revoked:
        # 排除依赖资源被其他资源引用并授权的情况
        exclude_resource_cited_by_others_and_authorized(dependency_resources_uuids, resource_uuid_list, session)

    dependency_resources_uuids = filter_shared_agent(dependency_resources_uuids, session)
    dependency_resources = session.query(ResourceTable).filter(ResourceTable.uuid.in_(dependency_resources_uuids)).all()
    # 检查依赖资源是否已经保护
    check_all_resource_already_be_protected(dependency_resources_uuids, session)
    # 添加agent下的子资源
    dependency_resources_uuids.extend(append_agent_sub_resources(dependency_resources, session))
    log.debug(f"dependency resources uuids :{dependency_resources_uuids}")
    return dependency_resources_uuids + resource_uuid_list


def exclude_resource_cited_by_others_and_authorized(dependency_resources_uuids: List,
                                                    resource_uuid_list, session):
    """

    Args:
        dependency_resources_uuids:  依赖的资源id
        resource_uuid_list:  传入的要取消授权的资源

    Returns:

    """
    condition = {
        ResExtendInfoTable.key.like(ResourceConstants.CITATION + ResourceConstants.CITATION_SEPERATOR + '%'),
        ResExtendInfoTable.value.in_(resource_uuid_list)
    }
    all_dependency_resources_uuids = session.query(ResExtendInfoTable.resource_id).filter(*condition).all()
    if all_dependency_resources_uuids is None or len(all_dependency_resources_uuids) == 0:
        return
    all_dependency_resources_uuids = list(set(i[0] for i in all_dependency_resources_uuids))
    extend_condition = {
        ResExtendInfoTable.resource_id.in_(all_dependency_resources_uuids)
    }
    dependency_resource_extends = session.query(ResExtendInfoTable).filter(*extend_condition).all()
    other_cite_resource_uuids = []
    for extend in dependency_resource_extends:
        if extend.key is not None and extend.key.startswith(
                ResourceConstants.CITATION) and extend.value not in resource_uuid_list:
            other_cite_resource_uuids.append(extend.value)
    if len(other_cite_resource_uuids) == 0:
        return
    other_cite_resources = session.query(ResourceTable).filter(ResourceTable.uuid.in_(other_cite_resource_uuids)).all()
    other_cite_resource_user_map = {i.uuid: i.user_id for i in other_cite_resources}
    # 如果其他资源引用了dependency_resources_uuids，则该资源不能移除授权
    for extend in dependency_resource_extends:
        user_id = other_cite_resource_user_map.get(extend.value)
        if user_id is not None and len(user_id) > 0 and extend.resource_id in dependency_resources_uuids:
            dependency_resources_uuids.remove(extend.resource_id)


def append_agent_sub_resources(dependency_resources, session):
    sub_resource_ids = []
    dispatcher = OwnershipDispatcher(ResourceSubTypeEnum.UBackupAgent.value)
    for resource in dependency_resources:
        if resource.sub_type == ResourceSubTypeEnum.UBackupAgent.value:
            children_uuids = dispatcher.query_subset_id_list(resource, session)
            sub_resource_ids.extend(children_uuids)
    return sub_resource_ids


def check_all_resources_already_be_authorized(resource_uuid_list: List[str], user_id, session):
    count = session.query(ResourceTable.uuid).filter(
        ResourceTable.uuid.in_(resource_uuid_list)).filter(
        ResourceTable.user_id.isnot(None)).filter(ResourceTable.user_id != user_id).count()
    if count > 0:
        raise EmeiStorBizException(UserErrorCodes.RESOURCE_BEEN_AUTHORIZED)


def check_all_resource_already_be_protected(resource_uuid_list: List[str], session):
    for resource_uuid in resource_uuid_list:
        OwnershipDispatcher.is_sla_bounded(resource_uuid, session)


def revoke(user_id, resource_uuid_list: List[str]):
    # 检验资源列表里面的资源参数
    if resource_uuid_list is None or user_id is None:
        raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, message="params is illegal")
    for resource_uuid in resource_uuid_list:
        if len(resource_uuid) > 512:
            raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, message="params is illegal")
    with database.session() as session:
        # 检查资源是否已被其他资源依赖
        check_resource_is_depended_on(resource_uuid_list, session)
        authorize_resource_list = query_authorize_resource_list(resource_uuid_list, session)
        authorize_resource_list = search_citation_resource_uuid(authorize_resource_list, session, True)
        session.query(ResourceTable).filter(ResourceTable.uuid.in_(authorize_resource_list)).update(
            {ResourceTable.user_id: None, ResourceTable.authorized_user: None}, synchronize_session=False)


def is_need_check_dependency(dependent_resources_uuids, res_extend_info_resources, session):
    resource_list = session.query(ResourceTable).filter(ResourceTable.uuid.in_(dependent_resources_uuids)).all()
    for resource in resource_list:
        for res_extend_info in res_extend_info_resources:
            # 由于以下3个特性接入有问题，资源的parent_uuid就是主机的id并且依赖主机,导致循坏依赖这里做规避手段
            if res_extend_info.resource_id == resource.parent_uuid and res_extend_info.value == resource.uuid and \
                    resource.sub_type in [ResourceSubTypeEnum.SQLServerInstance.value,
                                          ResourceSubTypeEnum.MysqlInstance.value,
                                          ResourceSubTypeEnum.PostgreInstance.value]:
                return True
    return False


def check_resource_is_depended_on(resource_uuid_list, session):
    condition = {
        ResExtendInfoTable.key.like(ResourceConstants.CITATION + ResourceConstants.CITATION_SEPERATOR + '%'),
        ResExtendInfoTable.resource_id.in_(resource_uuid_list)
    }
    res_extend_info_resources = session.query(ResExtendInfoTable).filter(*condition).all()
    dependent_resources_uuids = [res_extend_info.value for res_extend_info in res_extend_info_resources]
    if is_need_check_dependency(dependent_resources_uuids, res_extend_info_resources, session):
        return
    if dependent_resources_uuids:
        log.error(f"The resource is already occupied by {dependent_resources_uuids}")
        param = dependent_resources_uuids
        res = session.query(ResourceTable).filter(ResourceTable.uuid.in_(dependent_resources_uuids)).first()
        if res:
            param = res.name
            # 数据库实例依赖agent场景
            if not res.name:
                cluster = session.query(ResourceTable).filter(ResourceTable.uuid == res.root_uuid).first()
                param = cluster.name
        raise EmeiStorBizException(UserErrorCodes.RESOURCE_BEEN_OCCUPIED, *[param],
                                   message="The resource is already occupied by another resource.")


def query_agent_info():
    with database.session() as session:
        query = session.query(ResourceTable)
        query = query.filter(ResourceTable.type == ResourceTypeEnum.Host.value,
                             ResourceTable.sub_type == ResourceSubTypeEnum.VMBackupAgent.value)
        if query.count() == 0:
            return {}
        else:
            return query.first()


def comment_event_message(topic: str, request_id: str = None, **kwargs):
    message = CommonEvent(topic=topic,
                          request_id=request_id,
                          **kwargs)
    producer.produce(message)


def query_resource_info(resource_list: List[str]):
    with database.session() as session:
        query = session.query(ResourceTable).filter(ResourceTable.uuid.in_(resource_list))
        return query.all()


def query_resource_group_info(resource_group_list: List[str]):
    with database.session() as session:
        query = session.query(ResourceGroup).filter(ResourceGroup.uuid.in_(resource_group_list))
        return query.all()


def update_protection_status(session, resource_id_list: List[str], protection_status: ProtectionStatusEnum,
                             is_resource_group: bool = False):
    """
    更新资源保护状态
    :param session:
    :param resource_id_list: 资源id列表
    :param protection_status: 保护状态
    :param is_resource_group: 是否是资源组
    :return:
    """
    table = ResourceGroup if is_resource_group else ResourceTable

    if protection_status is protection_status.protected:
        # 更新状态为已保护
        session.query(table).filter(
            table.uuid.in_(resource_id_list),
            table.protection_status.in_(
                [protection_status.unprotected.value, protection_status.protecting.value])).update(
            {table.protection_status: protection_status.protected.value},
            synchronize_session='fetch')
    elif protection_status is protection_status.protecting:
        # 更新状态为保护中
        session.query(table).filter(
            table.uuid.in_(resource_id_list),
            table.protection_status == protection_status.unprotected.value).update(
            {table.protection_status: protection_status.protecting.value},
            synchronize_session='fetch')
    else:
        # 更新状态为未保护
        session.query(table).filter(
            table.uuid.in_(resource_id_list),
            table.protection_status.in_(
                [protection_status.protected.value, protection_status.protecting.value])).update(
            {table.protection_status: protection_status.unprotected.value},
            synchronize_session='fetch')


def update_link_status(env_id_list: List[str], link_status: LinkStatusEnum):
    """
    更新资源保护状态
    :param env_id_list: 受保护环境列表
    :param link_status: 当前环境的连接状态
    :return:
    """
    with database.session() as session:
        if link_status is LinkStatusEnum.Online:
            session.query(EnvironmentTable).filter(
                EnvironmentTable.uuid.in_(env_id_list),
                EnvironmentTable.link_status == link_status.Offline.value).update(
                {EnvironmentTable.link_status: link_status.Online.value},
                synchronize_session=False)
        else:
            session.query(EnvironmentTable).filter(
                EnvironmentTable.uuid.in_(env_id_list),
                EnvironmentTable.link_status == link_status.Online.value).update(
                {EnvironmentTable.link_status: link_status.Offline.value},
                synchronize_session=False)


def revoke_resource_user_id(user_id):
    with database.session() as session:
        session.query(ResourceTable).filter(
            ResourceTable.user_id == user_id
        ).update({ResourceTable.user_id: None, ResourceTable.authorized_user: None})


def set_database_info(agent_info_dict):
    log.info(f"get agent info instName: {agent_info_dict.get('instName', '')}"
             f"state: {agent_info_dict.get('state', ResourceStatus.OFF_LINE.value)}")
    # 格式化agent数据
    return {
        'instName': agent_info_dict.get('instName', ''),
        'dbName': agent_info_dict.get('dbName', ''),
        'version': agent_info_dict.get('version', ''),
        'state': agent_info_dict.get('state', ResourceStatus.OFF_LINE.value),
        'isAsmInst': agent_info_dict.get('isAsmInst', 0),
        'authType': agent_info_dict.get('authType', AuthType.OS_OFF.value),
        'dbRole': agent_info_dict.get('dbRole', 0),
        'oracleHome': agent_info_dict.get('oracleHome', '')
    }


def instance_available(host_ip):
    with database.session() as db:
        log.info(f'[Get database]: {host_ip}')
        env_list = db.query(EnvironmentTable).filter(EnvironmentTable.endpoint == host_ip).all()
        if len(env_list) > 1:
            log.error(f'Multiple hosts are the same ip: {host_ip}')

        try:
            database_agent = hw_agent_rpc.query_databases(host_ip)
        except Exception as ex:
            raise EmeiStorBizException(ResourceErrorCodes.HOST_OFFLINE) from ex
        finally:
            pass

        return list(set_database_info(i) for i in database_agent)


def get_resource_by_name(resource_name: str):
    log.info(f"check resource_name: {resource_name} exist")
    with database.session() as session:
        return session.query(ImportResourceTable).filter(ImportResourceTable.name == resource_name).first()


def query_target_cluster_by_id(cluster_id):
    with database.session() as db:
        return db.query(TClusterTarget).filter(TClusterTarget.cluster_id == cluster_id).one_or_none()


def get_extend_info_by_resource_id(resource_id):
    """
    根据资源UUID查询扩展信息字典
    """
    with database.session() as session:
        extend_info_list = session.query(ResExtendInfoTable.key, ResExtendInfoTable.value).filter(
            ResExtendInfoTable.resource_id == resource_id).all()
    return {i[0]: i[1] for i in extend_info_list}


def query_local_cluster():
    with database.session() as db:
        return db.query(TClusterLocal).one_or_none()


def query_current_cluster(esn):
    with database.session() as db:
        return db.query(ClusterMemberTable).filter(ClusterMemberTable.remote_esn == esn).one_or_none()


def query_distribution_storage_by_id(uuid):
    with database.session() as db:
        return db.query(TDistributionStorage).filter(TDistributionStorage.uuid == uuid).one_or_none()


def query_res_extend_info_by_resource_id(res_id):
    with database.session() as session:
        exist_extend_infos = session.query(ResExtendInfoTable).filter(*{
            ResExtendInfoTable.resource_id == res_id}).all()
    return exist_extend_infos


def query_resource_group_member_info(resource_id):
    with database.session() as db:
        return db.query(ResourceGroupMember).filter(ResourceGroupMember.source_id == resource_id).one_or_none()


def query_label_info(resource_id):
    with database.session() as db:
        labels = (
            db.query(LabelTable)
            .join(LabelResourceTable, LabelTable.uuid == LabelResourceTable.label_id)
            .filter(LabelResourceTable.resource_object_id == resource_id)
            .all()
        )
        return labels


def get_domain_label_resources_mapping(db, resource_uuids, domain_id):
    domain_resource_ids = db.query(DomainResourceObjectTable.resource_object_id).filter(
        DomainResourceObjectTable.domain_id == domain_id
    ).all()
    domain_resource_ids = [resource_id[0] for resource_id in domain_resource_ids]
    label_resources = db.query(LabelResourceTable).filter(
        LabelResourceTable.resource_object_id.in_(resource_uuids),
        LabelResourceTable.label_id.in_(domain_resource_ids)
    ).all()
    label_resources_map = {}
    for lr in label_resources:
        if lr.resource_object_id not in label_resources_map:
            label_resources_map[lr.resource_object_id] = []
        label_resources_map[lr.resource_object_id].append(lr.label_id)
    all_label_ids = sum(label_resources_map.values(), [])
    labels = db.query(LabelTable).filter(LabelTable.uuid.in_(all_label_ids)).all()
    labels_map = {label.uuid: label.as_dict() for label in labels}

    return label_resources_map, labels_map


def get_label_resources_mapping(db, resource_uuids):
    label_resources = db.query(LabelResourceTable).filter(
        LabelResourceTable.resource_object_id.in_(resource_uuids)
    ).all()
    label_resources_map = {}
    for lr in label_resources:
        if lr.resource_object_id not in label_resources_map:
            label_resources_map[lr.resource_object_id] = []
        label_resources_map[lr.resource_object_id].append(lr.label_id)
    all_label_ids = [label_id
                     for label_ids in label_resources_map.values()
                     for label_id in label_ids]
    labels = db.query(LabelTable).filter(LabelTable.uuid.in_(all_label_ids)).all()
    labels_map = {label.uuid: label.as_dict() for label in labels}

    return label_resources_map, labels_map


def common_add_label_info_with_query_response(response, token):
    if not response or not response.items:
        return response

    with database.session() as db:
        resource_uuids = [item['uuid'] for item in response.items]

        # 如果 token 是 bytes 类型，获取用户信息并传递 domain_id
        if isinstance(token, bytes):
            user_info = get_user_info_from_token(token)
            domain_id = user_info.get("domain-id")
            label_resources_map, labels_map = get_domain_label_resources_mapping(db, resource_uuids, domain_id)
        else:
            # 否则，不传递 domain_id
            label_resources_map, labels_map = get_label_resources_mapping(db, resource_uuids)

        # 为每个 item 赋值 label_List，并将 LabelTable 对象转换为字典形式
        for item in response.items:
            resource_uuid = item['uuid']
            label_ids_for_item = label_resources_map.get(resource_uuid, [])
            item['labelList'] = [labels_map[label_id] for label_id in label_ids_for_item]

    return response


def get_user_name_mapping(db, user_id):
    if not user_id:
        return "--"

    user_name = db.query(UserTable.user_name).filter(
        UserTable.user_id == user_id
    ).first()

    if not user_name:
        log.info(f"User ID '{user_id}' not found in the database.")
        return "--"

    return user_name[0]


def common_add_label_json_info_with_query_response(response, token):
    if not response or not response.items:
        return response

    # 如果 token 是字节类型，获取用户信息和 domain_id
    domain_id = None
    if isinstance(token, bytes):
        user_info = get_user_info_from_token(token)
        domain_id = user_info.get("domain-id")

    with database.session() as db:
        resource_uuids = [item['uuid'] for item in response.items]
        # 根据是否存在 domain_id 调用不同的标签资源映射方法
        if domain_id:
            label_resources_map, labels_map = get_domain_label_resources_mapping(db, resource_uuids, domain_id)
        else:
            label_resources_map, labels_map = get_label_resources_mapping(db, resource_uuids)

        for item in response.items:
            resource_uuid = item['uuid']
            user_id = item['user_id']
            user_name = get_user_name_mapping(db, user_id)
            label_ids_for_item = label_resources_map.get(resource_uuid, [])
            labels_for_item = [labels_map[label_id] for label_id in label_ids_for_item]
            properties = json.loads(item.get('properties', '{}'))
            properties['userName'] = user_name
            properties['labelList'] = labels_for_item
            item['properties'] = json.dumps(properties, separators=(',', ':'))

    return response
