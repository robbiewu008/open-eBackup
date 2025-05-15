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
import os
import ast
import json

from datetime import datetime, timedelta
from zoneinfo import ZoneInfo
from typing import List, Dict
from dateutil.relativedelta import relativedelta

from sqlalchemy import func, false, and_, or_, true, extract
from sqlalchemy.dialects.postgresql import aggregate_order_by
from sqlalchemy.orm import Query

from app.archive.service.constant import ArchiveConstant
from app.backup.common.config import db_config
from app.common import logger
from app.common.clients.resource_client import ResourceClient
from app.common.constants.constant import CommonConstants
from app.common.context.db_session import Session
from app.common.deploy_type import DeployType
from app.common.enums.copy_enum import GenerationType
from app.common.enums.rbac_enum import ResourceSetTypeEnum
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import BackupTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exception.user_error_codes import UserErrorCodes
from app.common.log.kernel import load_resource, convert_storage_type
from app.common.schemas.common_schemas import UuidObject
from app.common.security.jwt_utils import get_user_info_from_token
from app.common.security.role_dict import RoleEnum
from app.common.util.time_util import get_local_timezone
from app.copy_catalog.common.copy_status import CopyStatus
from app.copy_catalog.copy_error_code import CopyErrorCode
from app.copy_catalog.models.req_param import NoArchiveReq
from app.copy_catalog.models.tables_and_sessions import database, CopyTable, CopyArchiveMapTable, \
    ReplicatedCopiesTable, ClusterMemberTable, StorageUnitTable
from app.copy_catalog.schemas import CopyStatisticView, CopyStatistic, CopyInfoSchema, CopySchema
from app.copy_catalog.util.copy_util import check_status, check_copy_status, get_latest_full_copy, \
    check_copy_name_not_include_invalid_character
from app.resource.models.label_models import LabelResourceTable, LabelTable
from app.resource.models.rbac_models import DomainResourceObjectTable, ResourceSetResourceObjectTable
from app.resource.models.resource_models import ResourceTable
from app.resource.service.common.user_domain_service import get_domain_id_by_user_id

log = logger.get_logger(__name__)


def _get_db_time(local_time):
    native_datetime = datetime.strptime(local_time, '%Y-%m-%d %H:%M:%S')
    beijing_tz = ZoneInfo(CommonConstants.DEFAULT_TIMEZONE)
    db_time = native_datetime.astimezone(beijing_tz)
    return db_time.strftime("%Y-%m-%d %H:%M:%S")


def get_year_start_end_time(year):
    return _get_db_time(year + "-01-01 00:00:00"), _get_db_time(year + "-12-31 23:59:59")


def get_month_start_end_time(month):
    date_obj = datetime.strptime(month + "-01", "%Y-%m-%d")
    last_day_of_month = date_obj + relativedelta(months=1) - timedelta(days=1)
    return _get_db_time(month + "-01 00:00:00"), _get_db_time(
        last_day_of_month.strftime("%Y-%m-%d") + " 23:59:59")


def get_week_start_end_time(week_str):
    week_sp = week_str.split("-")
    year = int(week_sp[0])
    week = int(week_sp[1])
    total_weeks_of_year = get_total_weeks(year)
    week = total_weeks_of_year if week > total_weeks_of_year else week
    approx_date = datetime(year, 1, 4)
    _, approx_week, _ = approx_date.isocalendar()
    days_to_move = (week - approx_week) * 7
    if days_to_move < 0:
        days_to_move = 0
    start_date = approx_date + timedelta(days=days_to_move - approx_date.weekday())  # 调整为周一
    end_date = start_date + timedelta(days=6)
    return _get_db_time(start_date.strftime("%Y-%m-%d 00:00:00")), _get_db_time(end_date.strftime("%Y-%m-%d 23:59:59"))


def get_total_weeks(year):
    # 获取该年最后一天的日期
    last_day_of_year = datetime(year, 12, 31)

    # 如果12月31日不属于该年的最后一周，要获取该年最后一周的周数
    if last_day_of_year.isocalendar()[0] != year:
        # 如果 12 月 31 日属于下一年的第一周，获取前一周
        last_day_of_year = last_day_of_year.replace(month=12, day=24)  # 返回上周的时间

    # 获取该年的最后一周的周数
    return last_day_of_year.isocalendar()[1]


def get_date_start_end_time(date):
    return _get_db_time(date + " 00:00:00"), _get_db_time(date + " 23:59:59")


def get_hour_start_end_time(hour):
    return _get_db_time(hour + ":00:00"), _get_db_time(hour + ":59:59")


def copy_authenticate(token):
    user_info = get_user_info_from_token(token)
    domain_id = user_info.get('domain-id')

    def initiator(query: Query) -> Query:
        with db_config.get_session() as session:
            sub_query = session.query(DomainResourceObjectTable.resource_object_id).filter(
                DomainResourceObjectTable.domain_id == domain_id).filter(
                DomainResourceObjectTable.type == ResourceSetTypeEnum.COPY.value).subquery()
            query = query.filter(CopyTable.uuid.in_(sub_query))
        return query

    return initiator


def copy_data_condition_filter(condition: Dict[str, any]):
    custom_initiator = None
    if not condition:
        return custom_initiator
    if 'userInfoForLabel' in condition:
        del condition['userInfoForLabel']
    year = condition.get("year")
    month = condition.get("month")
    week = condition.get("week")
    date = condition.get("date")
    hour = condition.get("hour")
    origin_date = condition.get("origin_date")
    starts_time = condition.get("starts_time")
    ends_time = condition.get("ends_time")
    cluster_name = condition.get("cluster_name")
    storage_unit_name = condition.get("storage_unit_name")
    resource_set_id = condition.get("resource_set_id")
    user_id = condition.get("user_id")
    gn_range = condition.get("gn_range")
    label_name = condition.get('labelName')
    label_list = condition.get('labelList')
    gn_range = json.loads(gn_range) if isinstance(gn_range, str) else gn_range
    gn_range = gn_range if isinstance(gn_range, (list, tuple)) else None

    def initiator(query: Query, session: Session) -> Query:
        """
            入参格式
            put("starts_time", "yyyy-MM-dd HH:mm:ss");
            put("ends_time", "yyyy-MM-dd HH:mm:ss");
            put("hour", "yyyy-MM-dd HH");
            put("date", "yyyy-MM-dd");
            put("week", "yyyy-w");
            put("month", "yyyy-MM");
            put("year", "yyyy");
        """
        query = _build_time_query(query)
        if cluster_name:
            query = query.filter(ClusterMemberTable.cluster_name.ilike(f'%{cluster_name}%', escape='#'))
        if storage_unit_name:
            query = query.filter(StorageUnitTable.name.ilike(f'%{storage_unit_name}%', escape='#'))
        if resource_set_id:
            sub_query = session.query(ResourceSetResourceObjectTable.resource_object_id).filter(
                ResourceSetResourceObjectTable.resource_set_id == resource_set_id).subquery()
            query = query.filter(CopyTable.uuid.in_(sub_query))
        if label_name:
            label_query = session.query(LabelResourceTable.resource_object_id) \
                .join(LabelTable, LabelResourceTable.label_id == LabelTable.uuid) \
                .filter(LabelTable.name.ilike(f"%{label_name}%")).subquery()
            query = query.filter(CopyTable.uuid.in_(label_query))
        if label_list:
            if isinstance(label_list, list) and all(isinstance(label, str) for label in label_list):
                resource_ids = session.query(LabelResourceTable.resource_object_id) \
                    .filter(LabelResourceTable.label_id.in_(label_list)) \
                    .group_by(LabelResourceTable.resource_object_id) \
                    .having(func.count(LabelResourceTable.label_id) == len(label_list)) \
                    .subquery()
                query = query.filter(CopyTable.uuid.in_(resource_ids))
        if user_id:
            query = get_domain_id_from_user_id(query, session, user_id)
        if isinstance(gn_range, (list, tuple)) and len(gn_range) == 2:
            items = list(gn for gn in gn_range if gn is not None)
            if items:
                if len(items) == 2:
                    query = query.filter(CopyTable.gn.between(*gn_range))
                elif gn_range[0] is not None:
                    query = query.filter(CopyTable.gn >= gn_range[0])
                elif gn_range[1] is not None:
                    query = query.filter(CopyTable.gn <= gn_range[1])
        return query

    def _build_time_query(query):
        if starts_time and ends_time:
            query = query.filter(CopyTable.display_timestamp.between(starts_time, ends_time))
        if year:
            start_time, end_time = get_year_start_end_time(year)
            query = query.filter(CopyTable.display_timestamp.between(start_time, end_time))
        if month:
            start_time, end_time = get_month_start_end_time(month)
            query = query.filter(CopyTable.display_timestamp.between(start_time, end_time))
        if week:
            start_time, end_time = get_week_start_end_time(week)
            query = query.filter(CopyTable.display_timestamp.between(start_time, end_time))
        if date:
            start_time, end_time = get_date_start_end_time(date)
            query = query.filter(CopyTable.display_timestamp.between(start_time, end_time))
        if origin_date:
            start_time, end_time = get_date_start_end_time(origin_date)
            query = query.filter(CopyTable.origin_copy_time_stamp.between(start_time, end_time))
        if hour:
            start_time, end_time = get_hour_start_end_time(hour)
            query = query.filter(CopyTable.display_timestamp.between(start_time, end_time))
        return query

    def get_domain_id_from_user_id(query, session, current_user_id):
        domain_id = get_domain_id_by_user_id(current_user_id)
        if domain_id:
            query = add_domain_id_query(domain_id, query, session)
        else:
            query = query.filter(CopyTable.user_id == current_user_id)
        log.info(f'build copy query, user_id:{current_user_id}, domainId:{domain_id}')
        return query

    def add_domain_id_query(domain_id, query, session):
        sub_query = session.query(DomainResourceObjectTable.resource_object_id).filter(
            DomainResourceObjectTable.domain_id == domain_id).filter(
            DomainResourceObjectTable.type == ResourceSetTypeEnum.COPY.value).subquery()
        query = query.filter(CopyTable.uuid.in_(sub_query))
        return query

    return initiator


def query_copy_statistics(
        view: CopyStatisticView,
        time_point: str,
        resource_id: str, role: str, domain_id: str):
    filter_format_mapping = {
        CopyStatisticView.YEAR: "YYYY",
        CopyStatisticView.MONTH: "YYYY-MM",
    }
    if time_point:
        index_format_mappings = {
            CopyStatisticView.YEAR: "YYYY-MM",
            CopyStatisticView.MONTH: "YYYY-MM-DD",
        }
    else:
        index_format_mappings = filter_format_mapping
    index_format = index_format_mappings[view]
    with database.session() as session:

        query: Query = session.query(
            func.to_char(
                CopyTable.display_timestamp.op('AT TIME ZONE')(CommonConstants.DEFAULT_TIMEZONE).op('AT TIME ZONE')(
                    get_local_timezone()), index_format).label('index'),
            func.count(CopyTable.uuid).label('count')).filter(CopyTable.deleted == false())
        if domain_id is not None:
            sub_query = session.query(DomainResourceObjectTable.resource_object_id).filter(
                DomainResourceObjectTable.domain_id == domain_id).filter(
                DomainResourceObjectTable.type == ResourceSetTypeEnum.COPY.value).subquery()
            query = query.filter(CopyTable.uuid.in_(sub_query))
        if time_point is not None:
            if CopyStatisticView.YEAR == view:
                start_time, end_time = get_year_start_end_time(time_point)
            if CopyStatisticView.MONTH == view:
                start_time, end_time = get_month_start_end_time(time_point)
            query = query.filter(CopyTable.display_timestamp.between(start_time, end_time))
        if resource_id is not None:
            query = query.filter(CopyTable.resource_id == resource_id)
        query = query.group_by('index').order_by('index')
        return list(CopyStatistic(index=item[0], count=item[1]) for item in query.all())


def resource_status_statistics(resource_id: str, role: str, user_id: str):
    with database.session() as session:
        query: Query = session.query(CopyTable.status, func.count(CopyTable.uuid)).filter(
            and_(CopyTable.deleted.is_(False),
                 or_(role == RoleEnum.ROLE_SYS_ADMIN.value, CopyTable.user_id == user_id)))
        if resource_id:
            query = query.filter(CopyTable.resource_id == resource_id)

        result = query.group_by('status').order_by('status').all()
        dic = {k: v for k, v in result}
        dic['Total'] = sum(dic.values())
        return dic


def get_resource_latest_copy(resource_id: str, generated_by: str):
    # 默认降序
    with database.session() as session:
        query: Query = session.query(
            func.array_agg(aggregate_order_by(CopyTable.uuid, CopyTable.display_timestamp.desc()))[1]).filter(
            CopyTable.resource_id == resource_id, CopyTable.deletable == true(),
            CopyTable.generated_by == generated_by).group_by(
            CopyTable.resource_id)
        result = query.one_or_none()
        return result[0] if result else None


def get_copy(copy_id: str, strict: bool):
    with database.session() as session:
        copy: CopyTable = session.query(CopyTable).filter(CopyTable.uuid == copy_id).one_or_none()
        exists = bool(copy)
        check_status(exists, strict, CommonErrorCodes.OBJ_NOT_EXIST, f"The copy '{copy_id}' is not exist")
    return copy


def get_deleting_copy(copy_id: str, strict: bool = False):
    copy_in_db = None
    with database.session() as session:
        copy: CopyTable = session.query(CopyTable).filter(
            CopyTable.uuid == copy_id).one_or_none()
        exists = bool(copy)
        check_status(exists, strict, CommonErrorCodes.OBJ_NOT_EXIST, f"The copy '{copy_id}' is not exist")
        if exists:
            check_copy_status(copy_id, copy.status, strict)
    if copy is not None:
        copy_in_db = CopyTable(**copy.as_dict())
    return copy_in_db


def get_protect_resource_by_id(copy_id):
    copy = query_copy_by_id(copy_id)
    resource_id = copy.resource_id
    res = get_log_data_cyber(resource_id)
    # 特别设置，解决kernel中获取的结果为单个数组问题
    res.append("cyber-array-true")
    return res


def get_detect_status_cyber_log_data(params):
    return get_protect_resource_by_id(params.get("copy_id"))


def get_update_retention_cyber_log_data(params):
    return get_protect_resource_by_id(params.get("copy_id"))


def get_delete_copy_cyber_log_data(params):
    return get_protect_resource_by_id(params.get("copy_id"))


def get_log_data_cyber(resource_id):
    # cyber 单个查询用户（{0}:{1}）对资源（存储设备名（{2}）、设备序列号（{3}）、设备类型（{4}）、租户名（{5}）、租户ID（{6}）、资源ID（{7}）、资源名（{8}））修改保护。
    resource = load_resource([resource_id])[0]
    res = []
    path = resource.get("path").split("/")
    res.append(path[1])
    res.append(resource.get("root_uuid"))
    res.append(convert_storage_type(path[0]))
    res.append(resource.get("parent_name"))
    res.append(resource.get("parent_uuid"))
    res.append(resource.get("uuid"))
    res.append(resource.get("name"))
    return res


def query_copy_by_id(copy_id: str):
    with database.session() as session:
        return session.query(CopyTable).filter(CopyTable.uuid == copy_id).one_or_none()


def check_copy_whether_exist(copy_info: CopyInfoSchema, session):
    # 如果副本uuid在副本已存在返回副本uuid
    if copy_info.uuid is not None:
        copy_obj_exist = session.query(CopyTable).filter(
            CopyTable.uuid == copy_info.uuid
        ).one_or_none()
    elif copy_info.generated_by == "CloudArchive" and \
            copy_info.resource_sub_type != ResourceSubTypeEnum.Fileset.value:
        log.info(f'start to query archive copy')
        copy_obj_exist = session.query(CopyTable).filter(CopyTable.uuid == copy_info.uuid).first()
    else:
        copy_obj_exist = session.query(CopyTable).filter(
            and_(CopyTable.resource_id == copy_info.resource_id,
                 CopyTable.generated_by == copy_info.generated_by,
                 CopyTable.timestamp == copy_info.timestamp)).first()
    if copy_obj_exist:
        log.warn(f"Copy obj already exist"
                 f"resource_id = {copy_info.resource_id}, "
                 f"generated_by = {copy_info.generated_by},"
                 f"copy_id = {copy_obj_exist.uuid},"
                 f"timestamp = {copy_info.timestamp} already exist.")
    return copy_obj_exist


def verify_copy_ownership(user_id: str, copy_uuid_list: List[str]):
    if not copy_uuid_list:
        return
    if not user_id:
        raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)
    with database.session() as session:
        count = session.query(CopyTable.uuid).filter(CopyTable.uuid.in_(copy_uuid_list)).filter(
            CopyTable.user_id == user_id).count()
        if count != len(copy_uuid_list):
            raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)


def query_copy_by_condition(copy_id, sorted_key, conditions):
    with database.session() as session:
        copy_obj = session.query(CopyTable).filter(CopyTable.uuid == copy_id).first()
        if sorted_key:
            query = session.query(CopyTable).filter(CopyTable.chain_id == copy_obj.chain_id).order_by(
                CopyTable.gn.asc())
        else:
            query = session.query(CopyTable).filter(CopyTable.chain_id == copy_obj.chain_id).order_by(
                CopyTable.gn.desc())
        return handle_copy_info_by_condition(copy_obj, query, conditions)


def handle_copy_info_by_condition(copy, query: Query, conditions):
    copy_in_db = None
    condition = {k: v for k, v in ast.literal_eval(conditions).items()} if conditions else {}
    if query.count() == 0:
        return copy_in_db
    # 获取最新的全量副本
    if condition.__contains__("lasted_full_copy"):
        return get_latest_full_copy(query)

    # 获取增量副本依赖的全量副本
    if condition.__contains__("full_copy_by_increment_copy"):
        condition['gn'] = copy.gn
        return get_full_copy_by_increment_copy(condition, query)

    # 获取全量副本对应的过期时间最晚的非永久增量副本
    if condition.__contains__("lasted_expiration_time_increment_copy_by_full_copy"):
        return get_latest_expiration_increment_copy_by_full_copy(condition, query)
    return copy_in_db


def get_full_copy_by_increment_copy(condition, query):
    copy_in_db = None
    query = query.filter(CopyTable.backup_type == BackupTypeEnum.full.value, CopyTable.gn < condition["gn"])
    copy = query.first()
    if copy:
        copy_in_db = CopySchema.from_orm(copy)
    return copy_in_db


def get_latest_expiration_increment_copy_by_full_copy(condition, query):
    copy_in_db = None
    # 获取该全量副本对应的下个全量副本
    next_full_copy_query = query.filter(CopyTable.backup_type == BackupTypeEnum.full.value,
                                        CopyTable.gn > condition["gn"]).order_by(CopyTable.gn.asc())
    next_full_copies = next_full_copy_query.all()
    if next_full_copies:
        copies = query.filter(CopyTable.backup_type.in_([BackupTypeEnum.cumulative_increment.value,
                                                         BackupTypeEnum.difference_increment.value])) \
            .filter(CopyTable.gn > condition["gn"], CopyTable.gn < next_full_copies[-1].gn).all()
    else:
        copies = query.filter(CopyTable.backup_type.in_([BackupTypeEnum.cumulative_increment.value,
                                                         BackupTypeEnum.difference_increment.value])) \
            .filter(CopyTable.gn > condition["gn"]).all()
    # 查找永久保留的副本
    retain_copy = list(copy for copy in copies if copy.expiration_time is None)
    if retain_copy:
        return CopySchema.from_orm(retain_copy[0])

    # 查找最晚过期的副本
    increment_copy = sorted(copies, key=lambda c: c.expiration_time, reverse=True)
    if increment_copy:
        return CopySchema.from_orm(increment_copy[0])
    return copy_in_db


def query_copy_by_resource_id(resource_id, generated_by):
    copy_in_db = None
    with database.session() as session:
        if generated_by is not None:
            copy_obj = session.query(CopyTable).filter(CopyTable.resource_id == resource_id,
                                                       CopyTable.generated_by == generated_by).first()
        else:
            copy_obj = session.query(CopyTable).filter(CopyTable.resource_id == resource_id).first()
        if copy_obj is not None:
            copy_in_db = CopyTable(**copy_obj.as_dict())
    return copy_in_db


def query_replicated_copy_by_resource_id(resource_id):
    copy_in_db = None
    with database.session() as session:
        copy_obj = session.query(CopyTable) \
            .filter(CopyTable.resource_id == resource_id,
                    CopyTable.generated_by.in_([GenerationType.BY_REPLICATED.value,
                                                GenerationType.BY_REVERSE_REPLICATION.value,
                                                GenerationType.BY_CASCADED_REPLICATION.value])).first()
        if copy_obj is not None:
            copy_in_db = copy_obj.as_dict()
    return copy_in_db


def query_copy_count_by_resource_id(resource_id):
    with database.session() as session:
        copy_count = session.query(CopyTable) \
            .filter(CopyTable.resource_id == resource_id).count()
    return copy_count


def query_copy_by_timestamp(timestamp_list: str, database_id: str):
    with database.session() as session:
        copies: List[CopyTable] = session.query(CopyTable).filter(
            CopyTable.timestamp.in_(timestamp_list),
            CopyTable.generated_by == 'Backup',
            CopyTable.resource_id == database_id).all()
        return copies


def query_no_archive_copy_list(param: NoArchiveReq):
    resource_id = param.resource_id
    copy_id = param.copy_id
    generated_by = param.generated_by
    user_id = param.user_id
    query_condition = [CopyArchiveMapTable.storage_id == param.storage_id,
                       CopyArchiveMapTable.resource_id == resource_id]
    copytable_query_condition = [CopyTable.resource_id == resource_id]
    if copy_id is not None:
        query_condition.append(CopyArchiveMapTable.copy_id == copy_id)
        copytable_query_condition.append(CopyTable.uuid == copy_id)
    if generated_by is not None:
        copytable_query_condition.append(CopyTable.generated_by == generated_by)
    if user_id is not None:
        copytable_query_condition.append(CopyTable.user_id == user_id)
    if not param.is_query_log_copy:
        copytable_query_condition.append(CopyTable.backup_type != BackupTypeEnum.log.value)
    with database.session() as session:
        copy_map = session.query(CopyArchiveMapTable.copy_id, CopyArchiveMapTable.storage_id).filter(
            *query_condition).subquery()
        copies: List[CopyTable] = session.query(CopyTable).filter(*copytable_query_condition).outerjoin(
            copy_map, CopyTable.uuid == copy_map.c.copy_id).filter(copy_map.c.copy_id.is_(None)).all()
        return list(CopySchema.from_orm(copy) for copy in copies)


def query_copy_backup_id(copy_id: str):
    with database.session() as session:
        copy = session.query(CopyTable).filter(
            CopyTable.uuid == copy_id).one_or_none()
        if copy is None:
            return ""
        properties = json.loads(copy.properties)
        return properties.get("backup_id")


def query_delete_copy_list(chain_id):
    with database.session() as session:
        copies = session.query(CopyTable).filter(
            CopyTable.chain_id == chain_id,
            CopyTable.deleted == true()).order_by(CopyTable.gn.desc()).all()
        return copies


def query_replicated_copies(copy_id, esn):
    with database.session() as session:
        query = session.query(ReplicatedCopiesTable)
        if copy_id:
            query = query.filter(ReplicatedCopiesTable.copy_id == copy_id)
        if esn:
            query = query.filter(ReplicatedCopiesTable.esn == esn)
        return query.all()


def query_archive_copy_count_by_storage_id(storage_id):
    with database.session() as session:
        filters = (
            GenerationType.BY_CLOUD_ARCHIVE.value,
            GenerationType.BY_IMPORTED.value,
            GenerationType.BY_BACKUP.value
        )
        count = session.query(func.count(CopyTable.uuid)).filter(
            and_(CopyTable.generated_by.in_(filters),
                 CopyTable.properties.like(f'%{storage_id}%'))).scalar()
        log.info(f"archive copy count={count}.")
        return count


def query_copy_by_resource_properties(root_uuid, generate_by, resource_sub_type):
    with database.session() as session:
        filters = {
            CopyTable.resource_properties.like(f'%"root_uuid": "{root_uuid}"%'),
            CopyTable.generated_by == generate_by, CopyTable.resource_sub_type == resource_sub_type
        }
        copy_obj = session.query(CopyTable).filter(*filters).first()
        log.info(
            f"query copy by resource properties root_uuid={root_uuid},copy_obj={copy_obj.uuid if copy_obj else None}")
        if copy_obj:
            copy_obj = CopySchema(**copy_obj.as_dict())
    return copy_obj


def query_copy_info_by_backup_id(origin_backup_id, esn, generated_by_list):
    type_list = list(generated_by.value for generated_by in generated_by_list)
    log.info(f"origin_backup_id:{origin_backup_id}, esn:{esn}, generated_by_list:{generated_by_list}")
    if DeployType().is_dependent():
        copy_obj = query_copy_by_esn(esn, origin_backup_id, type_list)
    else:
        with database.session() as session:
            filters = {
                CopyTable.origin_backup_id == origin_backup_id, CopyTable.device_esn == esn,
                CopyTable.generated_by.in_(type_list)
            }
            copy_obj = session.query(CopyTable).filter(*filters).first()
            log.info(
                f"query copy by backup_id={origin_backup_id},esn={esn}")
            if copy_obj:
                copy_obj = CopySchema(**copy_obj.as_dict())
    return copy_obj


def query_copy_by_esn(esn, origin_backup_id, type_list):
    with database.session() as session:
        filters = {
            CopyTable.origin_backup_id == origin_backup_id,
            CopyTable.generated_by.in_(type_list)
        }
        copy_list = session.query(CopyTable).filter(*filters).all()
        log.info(
            f"query copy by backup_id={origin_backup_id}, esn={esn}, copy_list len:{len(copy_list)}")
        copy_tmp_list = []
        copy_esn_list = []
        copy_obj = None
        for copy in copy_list:
            if json.loads(copy.properties).get("repositories", [{}])[0] \
                    .get("extendInfo", {}).get("storage_info", {}).get("storage_device", "") == esn:
                copy_tmp_list.append(copy)
            if copy.device_esn == esn:
                copy_esn_list.append(copy)
        if copy_tmp_list:
            copy_obj = CopySchema(**copy_tmp_list[0].as_dict())
            return copy_obj
        if copy_esn_list:
            copy_obj = CopySchema(**copy_esn_list[0].as_dict())
            return copy_obj
    return copy_obj


def query_copy_info_by_uuid_and_esn(uuid, esn):
    with database.session() as session:
        filters = {CopyTable.uuid == uuid, CopyTable.device_esn == esn}
        copy_obj = session.query(CopyTable).filter(*filters).first()
        log.info(
            f"query copy by uuid={uuid},esn={esn}")
        if copy_obj:
            copy_obj = CopySchema(**copy_obj.as_dict())
    return copy_obj


def query_resource_type_name(params):
    resource_id = params.get("resource_id")
    resource = query_resource_by_resource_id(resource_id)
    if resource:
        return resource.sub_type, resource.name
    else:
        copy = query_copy_by_resource_id(resource_id, generated_by=None)
        if copy:
            return copy.resource_sub_type, copy.resource_name
    return '--', '--'


def query_deleted_copy_subtype(params):
    copy_id = params.get("copy_id")
    copy = get_deleting_copy(copy_id, True)
    if copy is None:
        return ""
    # 防勒索部署形态修改subtype名称
    if os.getenv("DEPLOY_TYPE") == 'd4' and copy.resource_sub_type == "CloudBackupFileSystem":
        return "FileSystem"
    return copy.resource_sub_type


def query_resource_by_resource_id(resource_id):
    with database.session() as session:
        resource_obj = session.query(ResourceTable).filter(ResourceTable.uuid == resource_id).first()
        return resource_obj


def check_copy_name_valid(copy_name: str):
    """
    根据副本名查副本
    :param copy_name: 副本名
    :return: 副本信息
    """
    check_copy_name_not_include_invalid_character(copy_name)
    with database.session() as session:
        copy: CopyTable = session.query(CopyTable).filter(CopyTable.name == copy_name).first()
        if copy is not None:
            raise_duplicate_copy_or_snapshot_name_exception(copy_name)


def raise_duplicate_copy_or_snapshot_name_exception(copy_name: str):
    if DeployType().is_cyber_engine_deploy_type():
        raise EmeiStorBizException(error=CopyErrorCode.FORBID_DUPLICATE_SNAPSHOT_NAME,
                                   message=f"snapshot name [{copy_name}] can not duplicate.")
    else:
        raise EmeiStorBizException(error=CopyErrorCode.FORBID_DUPLICATE_COPY_NAME,
                                   message=f"copy name [{copy_name}] can not duplicate.")


def count_copies_by_condition(resource_id: str, generated_by: str):
    with database.session() as session:
        conditions = (
            CopyTable.resource_id == resource_id,
            CopyTable.generated_by == generated_by
        )
        copies_count = session.query(CopyTable.uuid).filter(*conditions).count()
        return copies_count


def query_first_last_copy_id(resource_id: str, generated_by: str, month, year,
                             order_by=CopyTable.display_timestamp.desc()):
    """
    查询月的最新或第一个副本
    :param resource_id: 资源id
    :param generated_by: 生成方式
    :param month: 月份
    :param year: 年
    :param order_by: 排序方式，默认降序，返回最新副本
    :return: 副本id
    """
    with database.session() as session:
        filters = (
            CopyTable.resource_id == resource_id,
            CopyTable.deletable == true(),
            CopyTable.generated_by == generated_by,
            and_(extract("year", CopyTable.generated_time) == year,
                 extract("month", CopyTable.generated_time) == month)
        )
        query: Query = session.query(func.array_agg(aggregate_order_by(CopyTable.uuid, order_by))[1]). \
            filter(*filters).group_by(
            CopyTable.resource_id)
        result = query.one_or_none()
        return result[0] if result else None


def count_copy_by_parent_id(copy_id: str):
    """
    过滤条件，父副本id，生成方式，副本状态挂载中
    :param copy_id: 父副本id
    :return: 数量
    """
    with database.session() as session:
        live_mount_copy_status = (CopyStatus.MOUNTING.value, CopyStatus.MOUNTED.value, CopyStatus.UNMOUNTING.value)
        filters = (
            CopyTable.parent_copy_uuid == copy_id,
            CopyTable.generated_by == GenerationType.BY_LIVE_MOUNTE.value,
            CopyTable.status.in_(live_mount_copy_status)
        )
        count = session.query(func.count(CopyTable.uuid)).filter(*filters).scalar()
        log.info(f"copy({copy_id}) had mounted({count}).")
        return count


def get_same_chain_copies(copy_info: CopyTable) -> list:
    """
    获取副本依赖链上所有副本id（当前副本到依赖的第一个全量副本）
    如果当前副本为全量副本，则直接返回当前副本id

    :param copy_info：当前副本信息
    """
    copy_list = [copy_info.uuid]
    if copy_info.backup_type == BackupTypeEnum.full.value:
        return copy_list
    with database.session() as session:
        filters = (
            CopyTable.resource_id == copy_info.resource_id,
            CopyTable.chain_id == copy_info.chain_id,
            CopyTable.origin_copy_time_stamp < copy_info.origin_copy_time_stamp,
            CopyTable.generated_by == copy_info.generated_by
        )
        full_copy_backup_time = session.query(CopyTable.origin_copy_time_stamp).filter(*filters,
                                                          CopyTable.backup_type == BackupTypeEnum.full.value).order_by(
            CopyTable.origin_copy_time_stamp.desc()).first()
        if not full_copy_backup_time:
            log.info(
                f"{copy_info.uuid} has no dependent full copy on chain: {copy_info.chain_id}"
                f" and origin_copy_time_stamp: {copy_info.origin_copy_time_stamp}.")
            return copy_list
        filtered_copy = session.query(CopyTable.uuid)\
            .filter(*filters, CopyTable.origin_copy_time_stamp >= full_copy_backup_time[0]).order_by(
            CopyTable.origin_copy_time_stamp.desc()).all()
        for result in filtered_copy:
            copy_list.append(*result)
        return copy_list


def query_all_copy_ids_by_resource_id(resource_id: str, generated_by: str) -> list:
    """
    获取资源下所有副本id

    :param resource_id 资源id
    :param generated_by 副本生成方式

    :return 副本id列表
    """
    with database.session() as session:
        return session.query(CopyTable.uuid).filter(CopyTable.resource_id == resource_id,
                                                    CopyTable.generated_by == generated_by).order_by(
            CopyTable.timestamp.asc()).all()


def query_last_copy_by_resource_id(resource_id, generated_by):
    copy_in_db = None
    with database.session() as session:
        if generated_by is not None:
            copy_obj = session.query(CopyTable).filter(CopyTable.resource_id == resource_id,
                                                       CopyTable.generated_by == generated_by).order_by(
                CopyTable.timestamp.desc()).first()
        else:
            copy_obj = session.query(CopyTable).filter(CopyTable.resource_id == resource_id).order_by(
                CopyTable.timestamp.desc()).first()
        if copy_obj is not None:
            copy_in_db = CopyTable(**copy_obj.as_dict())
    return copy_in_db


def query_user_related_copy_by_resource_id(resource_id: str, domain_id: str, index_status=None,
                                           generated_by=None) -> list:
    """
    获取用户域下指定资源的所有副本id

    :param resource_id 资源id
    :param domain_id 域id
    :param index_status 副本索引状态
    :param generated_by 副本生成方式

    :return 副本id列表
    """
    with database.session() as session:
        copies = session.query(CopyTable).filter(CopyTable.resource_id == resource_id)
        if index_status is not None:
            copies = copies.filter(CopyTable.indexed == index_status)
        if generated_by is not None:
            copies = copies.filter(CopyTable.generated_by.in_(generated_by))
        if domain_id is not None:
            sub_query = session.query(DomainResourceObjectTable.resource_object_id).filter(
                DomainResourceObjectTable.domain_id == domain_id).filter(
                DomainResourceObjectTable.type == ResourceSetTypeEnum.COPY).subquery()
            copies = copies.filter(CopyTable.uuid.in_(sub_query)).all()
        copy_id_list = []
        if copies is not None:
            copy_id_list = [copy.uuid for copy in copies]
        log.info(f'query_user_index_copy_by_resource_id, copies: {copy_id_list}')
        return copy_id_list
