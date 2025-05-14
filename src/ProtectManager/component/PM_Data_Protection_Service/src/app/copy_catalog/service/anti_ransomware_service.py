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
import math
import time
from datetime import datetime, timedelta
from typing import List
from dateutil.relativedelta import relativedelta

from redis.exceptions import LockError
from sqlalchemy import distinct, func, case, asc, desc, or_, and_, null

from app.common import logger
from app.common.clients.resource_client import ResourceClient
from app.common.clients.system_base_client import SystemBaseClient
from app.common.constants.constant import CommonConstants
from app.common.deploy_type import DeployType
from app.common.enums.anti_ransomware_enum import AntiRansomwareEnum, CopyDetectionResultPeriodEnum
from app.common.enums.copy_enum import GenerationType
from app.common.enums.rbac_enum import ResourceSetTypeEnum
from app.common.enums.resource_enum import ResourceSubTypeEnum, LinkStatusEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exception.user_error_codes import UserErrorCodes
from app.common.lock.lock import Lock
from app.common.lock.lock_manager import lock_manager
from app.common.schemas.common_schemas import BasePage
from app.common.security.jwt_utils import check_user_is_admin_or_audit, get_user_info_from_token, get_user_id, \
    get_user_id_from_user_info
from app.copy_catalog.client.dee_client import alarm_handler, delete_report
from app.copy_catalog.common.common import ResourceStatus
from app.copy_catalog.common.constant import AntiRansomwareCopyConstant
from app.copy_catalog.common.copy_status import CopyStatus
from app.copy_catalog.copy_error_code import CopyErrorCode
from app.copy_catalog.models.tables_and_sessions import CopyTable, database, CopyAntiRansomwareTable, \
    AntiRansomwarePolicyTable, AntiRansomwarePolicyResourceTable, ResourceAntiRansomwareTable
from app.copy_catalog.schemas import CopyAntiRansomwareReq, CopyAntiRansomwareReport, \
    CopyAntiRansomwareStatistics, CopyAntiRansomwareSummary, CopyAntiRansomware, CopyAntiRansomwareReportSchemas, \
    ModifyCopyAntiRansomwareDetectionStatusReq
from app.copy_catalog.schemas.copy_schemas import CopyAntiRansomwareQuery
from app.resource.common.constants import EnvironmentRemoveConstants
from app.resource.models.rbac_models import DomainResourceObjectTable
from app.resource.models.resource_models import ResourceTable, ResExtendInfoTable

log = logger.get_logger(__name__)

UPDATE_RESOURCE_DETECT = "anti_ransomware_update_resource_detect"
CYBER_ENGINE_SPECIAL_COPY_STATUS = [CopyStatus.DELETING, CopyStatus.DELETEFAILED, CopyStatus.RESTORING]


def query_copy_anti_ransomware(copy_anti_ransomware_query: CopyAntiRansomwareQuery):
    if copy_anti_ransomware_query.page_size == 0:
        return BasePage()
    with database.session() as session:
        query = session.query(CopyTable) \
            .outerjoin(CopyAntiRansomwareTable, CopyAntiRansomwareTable.copy_id == CopyTable.uuid) \
            .filter(CopyTable.resource_id == copy_anti_ransomware_query.resource_id)
        if copy_anti_ransomware_query.generated_by_list is not None:
            query = query.filter(CopyTable.generated_by.in_(copy_anti_ransomware_query.generated_by_list))
        if copy_anti_ransomware_query.copy_start_time is not None:
            query = query.filter(CopyTable.timestamp > copy_anti_ransomware_query.copy_start_time)
        if copy_anti_ransomware_query.status == AntiRansomwareEnum.UNDETECTED.value:
            query = query.filter(or_(CopyAntiRansomwareTable.copy_id.__eq__(None),
                                     CopyAntiRansomwareTable.status == AntiRansomwareEnum.UNDETECTED.value))
        else:
            query = query.filter(CopyAntiRansomwareTable.status == copy_anti_ransomware_query.status)
        count = query.count()
        if count == 0:
            return BasePage(items=[], total=0, pages=0, page_no=copy_anti_ransomware_query.page_no, page_size=0)

        items = query.order_by(CopyTable.timestamp.desc()).limit(copy_anti_ransomware_query.page_size) \
            .offset(copy_anti_ransomware_query.page_no * copy_anti_ransomware_query.page_size).all()
    return BasePage(
        items=items,
        total=count,
        pages=math.ceil(count / copy_anti_ransomware_query.page_size),
        page_no=copy_anti_ransomware_query.page_no,
        page_size=len(items))


def create_detection_reports(copy_id: str, detection_report: CopyAntiRansomwareReq):
    # 安全一体机 准备中作为检测中入库和展示
    if DeployType().is_cyber_engine_deploy_type() and detection_report.status == AntiRansomwareEnum.PREPARE:
        detection_report.status = AntiRansomwareEnum.DETECTING
    with database.session() as session:
        # 当副本已删除，dee写入报告结束
        copy = session.query(CopyTable).filter(CopyTable.uuid == copy_id).one_or_none()
        if copy is None:
            log.warning(f"create detection reports warn. the copy {copy_id} is not exists. ")
            return

        if detection_report.detection_start_time:
            detection_report.detection_start_time = time.strftime(CommonConstants.COMMON_DATE_FORMAT,
                                                                  time.strptime(detection_report.detection_start_time,
                                                                                CommonConstants.DEE_DATE_FORMAT))
        if detection_report.detection_end_time:
            detection_report.detection_end_time = time.strftime(CommonConstants.COMMON_DATE_FORMAT,
                                                                time.strptime(detection_report.detection_end_time,
                                                                              CommonConstants.DEE_DATE_FORMAT))
        if detection_report.generate_type is None:
            detection_report.generate_type = 'COPY_DETECT'

        update_copy_detect_table(copy, detection_report, session)
        # 安全一体机额外更新资源检测表
        if DeployType().is_cyber_engine_deploy_type():
            update_resource_detect_table_with_lock(copy, detection_report, session)


def modify_detection_status(detection_status_req: ModifyCopyAntiRansomwareDetectionStatusReq):
    log.info(f"start modify detection status")
    if detection_status_req.vstore_id is None and detection_status_req.copy_id is None \
            and detection_status_req.resource_id is None:
        log.warning("All params is null, update is over.")
        return
    with database.session() as session:
        # 在防勒索副本数据库中查询待更新状态的副本
        filters = [CopyAntiRansomwareTable.status == detection_status_req.old_status]
        if detection_status_req.vstore_id:
            filters.append(CopyAntiRansomwareTable.tenant_id == detection_status_req.vstore_id)
        if detection_status_req.copy_id:
            filters.append(CopyAntiRansomwareTable.copy_id == detection_status_req.copy_id)
        if detection_status_req.resource_id:
            copy_id_sub_query = session.query(CopyTable.uuid).filter(
                CopyTable.resource_id == detection_status_req.resource_id).subquery()
            filters.append(CopyAntiRansomwareTable.copy_id.in_(copy_id_sub_query))

        if detection_status_req.rollback_worm_status:
            # worm状态修改为未设置
            sub_query = session.query(CopyAntiRansomwareTable.copy_id).filter(and_(*filters)).subquery()
            session.query(CopyTable).filter(CopyTable.uuid.in_(sub_query)).update(
                {CopyTable.worm_status: 1}, synchronize_session=False)
            log.info(f"rollback worm status success")

        # 批量更新副本状态
        session.query(CopyAntiRansomwareTable).filter(and_(*filters)).update(
            {CopyAntiRansomwareTable.status: detection_status_req.new_status}, synchronize_session=False)
        log.info(f"modify detection status is over")


def update_resource_detect_table_with_lock(copy, detection_report, session):
    # 加锁防止 同一个资源的资源检测表被并发更新导致数据重复
    lock: Lock = lock_manager.get_lock(key=UPDATE_RESOURCE_DETECT + copy.resource_id)
    if lock.lock(timeout=EnvironmentRemoveConstants.LOCK_TIME_OUT,
                 blocking_timeout=EnvironmentRemoveConstants.LOCK_WAIT_TIME_OUT_RESOURCE_STATUS_UPDATE):
        try:
            update_resource_detect_table(copy, detection_report, session)
        finally:
            release_lock(lock)
    else:
        log.info(
            f"create_detection_reports copy_id: {copy.uuid}, resource_id: {copy.resource_id}, get lock failed.")


def release_lock(lock: Lock):
    try:
        if lock.is_locked():
            lock.unlock()
        else:
            log.warn("lock is not owned.")
    except LockError as error:
        log.error(f"lock release failed. {error.__cause__}")


def update_copy_detect_table(copy, detection_report, session):
    detection_duration = None
    if detection_report.detection_end_time and detection_report.detection_start_time:
        detection_duration = int(
            time.mktime(time.strptime(detection_report.detection_end_time, CommonConstants.COMMON_DATE_FORMAT)) -
            time.mktime(time.strptime(detection_report.detection_start_time, CommonConstants.COMMON_DATE_FORMAT)))
    copy_anti_ransomware = CopyAntiRansomwareTable(copy_id=copy.uuid,
                                                   status=detection_report.status,
                                                   model=detection_report.model,
                                                   detection_start_time=detection_report.detection_start_time,
                                                   detection_end_time=detection_report.detection_end_time,
                                                   detection_duration=detection_duration,
                                                   report=detection_report.report,
                                                   tenant_id=detection_report.tenant_id,
                                                   tenant_name=detection_report.tenant_name,
                                                   total_file_size=detection_report.total_file_size,
                                                   changed_file_count=detection_report.changed_file_count,
                                                   added_file_count=detection_report.added_file_count,
                                                   deleted_file_count=detection_report.deleted_file_count,
                                                   handle_detect_infect=False,
                                                   generate_type=detection_report.generate_type,
                                                   infected_file_count=detection_report.infected_file_count,
                                                   backup_software=detection_report.backup_software,
                                                   infected_file_detect_duration=detection_report.infected_file_detect_duration
                                                   )
    session.merge(copy_anti_ransomware)


def update_resource_detect_table(copy, detection_report, session):
    resource_id = copy.resource_id
    old_resource_detect_result = session.query(ResourceAntiRansomwareTable).filter(
        ResourceAntiRansomwareTable.resource_id == resource_id).one_or_none()
    if old_resource_detect_result is None:
        old_resource_detect_result = ResourceAntiRansomwareTable()
    resource_clear_result_status_list = [AntiRansomwareEnum.UNINFECTING.value, AntiRansomwareEnum.INFECTING.value,
                                         AntiRansomwareEnum.DETECTING.value]
    pre_resource_generate_time = copy.display_timestamp.strftime(CommonConstants.COMMON_DATE_FORMAT)
    old_resource_copy_time = old_resource_detect_result.copy_generated_time
    log.info(f"merge resource detect result, resource id {resource_id}, pre time {pre_resource_generate_time}, "
             f"old time {old_resource_copy_time}, input status {detection_report.status} ")
    if ((old_resource_copy_time is None) or (pre_resource_generate_time >= old_resource_copy_time)) \
            and (detection_report.status in resource_clear_result_status_list):
        log.info(f"execute merge resource detect result")
        merge_resource_detect = ResourceAntiRansomwareTable(
            id=old_resource_detect_result.id,
            resource_id=resource_id,
            status=detection_report.status,
            copy_id=copy.uuid,
            copy_generated_time=pre_resource_generate_time,
            total_file_size=detection_report.total_file_size,
            changed_file_count=detection_report.changed_file_count,
            added_file_count=detection_report.added_file_count,
            deleted_file_count=detection_report.deleted_file_count,
            copy_detected_time=detection_report.detection_end_time,
            infected_file_count=detection_report.infected_file_count,
            backup_software=detection_report.backup_software,
            generate_type=detection_report.generate_type
        )
        session.merge(merge_resource_detect)
    else:
        log.info(f"refresh resource detect result")
        refresh_resource_detect_table(resource_id, skip_copy_id=copy.uuid)


def refresh_resource_detect_table_with_lock(resource_id, first_generate=False, skip_copy_id=None):
    # 带锁更新资源检测表 防止 同一个资源的资源检测表被并发更新导致数据重复
    lock: Lock = lock_manager.get_lock(key=UPDATE_RESOURCE_DETECT + resource_id)
    if lock.lock(timeout=EnvironmentRemoveConstants.LOCK_TIME_OUT,
                 blocking_timeout=EnvironmentRemoveConstants.LOCK_WAIT_TIME_OUT):
        try:
            refresh_resource_detect_table(resource_id, first_generate, skip_copy_id)
            log.info(f'Success refreshing resource:{resource_id}')
        finally:
            release_lock(lock)
    else:
        log.info(
            f"refresh_resource_detect_table_with_lock resource_id: {resource_id}, get lock failed.")


def refresh_resource_detect_table(resource_id, first_generate=False, skip_copy_id=None):
    with database.session() as session:
        copy_display_status_list = [AntiRansomwareEnum.UNINFECTING.value, AntiRansomwareEnum.INFECTING.value,
                                    AntiRansomwareEnum.DETECTING.value]
        old_resource_detect_result = session.query(ResourceAntiRansomwareTable).filter(
            ResourceAntiRansomwareTable.resource_id == resource_id).one_or_none()
        if not first_generate and old_resource_detect_result is None:
            log.info(f"cannot find resource detect, skip refresh, resource_id:{resource_id}")
            return
        newest_copy_detect_result = session.query(CopyTable, CopyAntiRansomwareTable) \
            .join(CopyTable, CopyAntiRansomwareTable.copy_id == CopyTable.uuid) \
            .filter(CopyTable.resource_id == resource_id) \
            .filter(CopyAntiRansomwareTable.status.in_(copy_display_status_list))
        if skip_copy_id is not None:
            newest_copy_detect_result = newest_copy_detect_result.filter(CopyTable.uuid != skip_copy_id)
        newest_copy_detect_result = newest_copy_detect_result.order_by(CopyTable.display_timestamp.desc()).limit(
            1).one_or_none()
        if newest_copy_detect_result is None:
            log.info(f"cannot find detected copy of resource_id:{resource_id}, delete resource detect result")
            session.query(ResourceAntiRansomwareTable).filter(
                ResourceAntiRansomwareTable.resource_id == resource_id).delete()
        else:
            newest_copy = newest_copy_detect_result[0]
            newest_copy_detect = newest_copy_detect_result[1]
            log.info(f"find latest detected copy {newest_copy.uuid} of resource_id:{resource_id}, "
                     f"update resource detect result")
            merge_resource_detect = ResourceAntiRansomwareTable(
                id=None if old_resource_detect_result is None else old_resource_detect_result.id,
                resource_id=resource_id,
                status=newest_copy_detect.status,
                copy_id=newest_copy.uuid,
                copy_generated_time=newest_copy.display_timestamp,
                total_file_size=newest_copy_detect.total_file_size,
                changed_file_count=newest_copy_detect.changed_file_count,
                added_file_count=newest_copy_detect.added_file_count,
                deleted_file_count=newest_copy_detect.deleted_file_count,
                copy_detected_time=newest_copy_detect.detection_end_time,
                infected_file_count=newest_copy_detect.infected_file_count,
                backup_software=newest_copy_detect.backup_software,
                generate_type=newest_copy_detect.generate_type
            )
            session.merge(merge_resource_detect)


def query_anti_ransomware_report(token: str, copy_id: str):
    # 如果是系统管理员则都可以查看
    user = get_user_info_from_token(token)
    with database.session() as session:
        copy = session.query(CopyTable).filter(CopyTable.uuid == copy_id)
        if not check_user_is_admin_or_audit(user):
            copy = copy.filter(CopyTable.user_id == get_user_id_from_user_info(user))
        copy = copy.one_or_none()
        if copy is None:
            log.warning(f"Query anti-ransomware report is warn. The copy {copy_id} is not exist.")
            raise EmeiStorBizException(CopyErrorCode.COPY_NOT_EXIST)
        anti_ransomware_report = session.query(CopyAntiRansomwareTable). \
            filter(CopyAntiRansomwareTable.copy_id == copy_id,
                   CopyAntiRansomwareTable.status > AntiRansomwareEnum.DETECTING.value).first()
        if anti_ransomware_report is None:
            log.warning(f"Query anti-ransomware report is warn. "
                        f"The copy {copy_id} anti-ransomware report is not exist.")
            raise EmeiStorBizException(CopyErrorCode.ANTI_RANSOMWARE_REPORT_NOT_EXIST)
        return CopyAntiRansomwareReport(copy_id=copy_id,
                                        timestamp=copy.display_timestamp
                                        .strftime(CommonConstants.COMMON_DATE_FORMAT),
                                        model=anti_ransomware_report.model,
                                        status=anti_ransomware_report.status,
                                        detection_duration=anti_ransomware_report.detection_duration,
                                        detection_time=anti_ransomware_report.detection_start_time,
                                        report=anti_ransomware_report.report,
                                        infected_file_detect_duration=anti_ransomware_report.infected_file_detect_duration
                                        )


def modify_anti_ransomware_copy(token: str, copy_id: str, ext_parameters=None):
    with database.session() as session:
        copy = session.query(CopyTable).filter(CopyTable.uuid == copy_id).one_or_none()
        if copy is None:
            log.error(f"modify anti-ransomware copy error. the copy {copy_id} is not exist.")
            raise EmeiStorBizException(CopyErrorCode.COPY_NOT_EXIST)

        user = get_user_info_from_token(token)
        if not check_user_is_admin_or_audit(user) and get_user_id_from_user_info(user) != copy.user_id:
            log.error(
                f"modify anti-ransomware copy error. the user {get_user_id(token)} can not modify copy {copy_id} .")
            raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)

        copy_anti_ransomware = session.query(CopyAntiRansomwareTable) \
            .filter(CopyAntiRansomwareTable.copy_id == copy_id).first()
        if copy_anti_ransomware is None:
            log.warning(f"modify anti-ransomware copy error. the copy {copy_id} anti-ransomware report is not exist.")
            raise EmeiStorBizException(CopyErrorCode.ANTI_RANSOMWARE_REPORT_NOT_EXIST)
        if copy_anti_ransomware.status != AntiRansomwareEnum.INFECTING.value:
            log.warning(f"modify anti-ransomware copy error. "
                        f"the copy {copy_id} is not infecting.")
            raise_copy_or_snapshot_not_infected_exception()
        alarm_handler([copy_id], copy_anti_ransomware.generate_type, ext_parameters)
    refresh_resource_detect_table_with_lock(copy.resource_id)
    with database.session() as session:
        session.query(CopyAntiRansomwareTable) \
            .filter(CopyAntiRansomwareTable.copy_id == copy_id).update({'handle_detect_infect': True})


def raise_copy_or_snapshot_not_infected_exception():
    if DeployType().is_cyber_engine_deploy_type():
        raise EmeiStorBizException(CopyErrorCode.SNAPSHOT_NOT_INFECTED)
    else:
        raise EmeiStorBizException(CopyErrorCode.COPY_NOT_INFECTED)


def query_anti_ransomware_copies_resource(tuple_param):
    check_deploy_type_support()
    token, resource_sub_type, page_no, page_size, orders, conditions = tuple_param
    if page_size == 0:
        return BasePage()
    # 验证排序参数
    check_anti_ransomware_copies_resource_orders(orders)
    user = get_user_info_from_token(token)
    page_query_params = {
        "page_no": page_no,
        "page_size": page_size
    }
    with database.session() as session:
        # 获得已绑定防勒索策略资源列表
        sub_query = query_anti_ransomware_copies_resource_subquery(
            session, resource_sub_type.split(','), conditions, user)
        count = sub_query.count()
        if count == 0:
            return BasePage(items=[], total=0, pages=0, page_no=page_no, page_size=0)

        # 查询资源检测统计列表
        subquery = sub_query.subquery()
        copies_statistics, count = query_anti_ransomware_copies_resource_data(
            session, subquery, page_query_params, convert_conditions(conditions), orders)

    items = convert_anti_ransomware_copies_resource_to_statistics(copies_statistics)
    return BasePage(
        items=items,
        total=count,
        pages=math.ceil(count / page_size),
        page_no=page_no,
        page_size=len(items))


def check_anti_ransomware_copies_resource_orders(orders: str):
    if orders:
        start_with = orders[0]
        order_by = orders[1:]
        start_with_dic = ["+", "-"]
        order_by_dic = ["total_copy_num", "uninspected_copy_num", "prepare_copy_num", "detecting_copy_num",
                        "uninfected_copy_num", "infected_copy_num", "abnormal_copy_num", "latest_detection_time",
                        "display_timestamp"]
        if start_with not in start_with_dic or order_by not in order_by_dic:
            raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS,
                                       message=f"orders (" + orders + ") illegal.")


def query_anti_ransomware_copies_resource_subquery(session, resource_sub_type_list: List[str], conditions, user):
    conditions = convert_conditions(conditions)
    # 有关联策略的资源
    anti_resource = session.query(
        AntiRansomwarePolicyResourceTable.resource_id.label("resource_id"),
        AntiRansomwarePolicyResourceTable.policy_id.label("policy_id"),
    )
    # 有检测副本的资源
    ransomware_resource = session.query(CopyTable.resource_id,
                                        AntiRansomwarePolicyResourceTable.policy_id,
                                        ) \
        .outerjoin(CopyAntiRansomwareTable, CopyAntiRansomwareTable.copy_id == CopyTable.uuid) \
        .outerjoin(AntiRansomwarePolicyResourceTable,
                   AntiRansomwarePolicyResourceTable.resource_id == CopyTable.resource_id)
    # 云备份兼容查询只有副本，没有勒索检测的情况
    # 备份软件如果没有绑定防勒索策略，只查询检测中/未感染/已感染/异常的资源,资源需要存在或者是复制副本的资源
    if ResourceSubTypeEnum.CloudBackupFileSystem.value not in resource_sub_type_list and \
            ResourceSubTypeEnum.LUN.value not in resource_sub_type_list:
        ransomware_resource = ransomware_resource.filter(CopyAntiRansomwareTable.copy_id.isnot(None)) \
            .filter(CopyAntiRansomwareTable.status.in_(AntiRansomwareCopyConstant.DETECTED_STATUS_LIST)) \
            .filter(or_(CopyTable.resource_status == ResourceStatus.EXIST.value,
                    CopyTable.generated_by.in_(AntiRansomwareCopyConstant.COPY_GENERATED_BY_REPLICATED_LIST)))

    resource_list = anti_resource.union(ransomware_resource).subquery()
    # 获得已绑定防勒索策略资源列表
    sub_query = get_resource_list_with_anti_policy(resource_list, resource_sub_type_list, session)

    if not check_user_is_admin_or_audit(user):
        sub_query = sub_query.filter(CopyTable.user_id == get_user_id_from_user_info(user))

    sub_query = query_resource_filter(conditions, sub_query)
    resource_sub = sub_query.group_by(CopyTable.resource_id).subquery()
    resource_with_tenant_list = session.query(resource_sub.c.resource_id, resource_sub.c.resource_sub_type,
                                              resource_sub.c.tenant_name, resource_sub.c.tenant_id,
                                              resource_sub.c.policy_name, resource_sub.c.policy_id)
    tenant_name = conditions.get('tenant_name', '')
    if tenant_name:
        resource_with_tenant_list = resource_with_tenant_list.filter(
            resource_sub.c.tenant_name.ilike(f'%{tenant_name}%', escape='#'))
    return resource_with_tenant_list


def get_resource_list_with_anti_policy(resource_list, resource_sub_type_list, session):
    sub_query = session.query(CopyTable.resource_id, func.max(CopyTable.resource_sub_type).label("resource_sub_type"),
                              func.max(
                                  case([(ResExtendInfoTable.key == 'tenantName', ResExtendInfoTable.value)],
                                       else_="")).label('tenant_name'),
                              func.max(
                                  case([(ResExtendInfoTable.key == 'tenantId', ResExtendInfoTable.value)],
                                       else_="")).label('tenant_id'),
                              func.max(AntiRansomwarePolicyTable.policy_name).label("policy_name"),
                              func.max(AntiRansomwarePolicyTable.id).label("policy_id")) \
        .outerjoin(ResExtendInfoTable, CopyTable.resource_id == ResExtendInfoTable.resource_id) \
        .join(resource_list,
              resource_list.c.resource_id == CopyTable.resource_id) \
        .outerjoin(AntiRansomwarePolicyTable,
                   AntiRansomwarePolicyTable.id == resource_list.c.policy_id) \
        .filter(CopyTable.resource_sub_type.in_(resource_sub_type_list)) \
        .filter(CopyTable.generated_by.in_(AntiRansomwareCopyConstant.SUPPORT_ANTI_COPY_GENERATED_BY_LIST)) \
        .filter(or_(CopyTable.resource_status == ResourceStatus.EXIST.value,
                    CopyTable.generated_by.in_(AntiRansomwareCopyConstant.COPY_GENERATED_BY_REPLICATED_LIST)))
    # 备份软件需要过滤只开了worm没有开detect的资源
    if not DeployType().is_cyber_engine_deploy_type() and not DeployType().is_hyper_detect_deploy_type():
        sub_query = sub_query.filter(~and_(AntiRansomwarePolicyTable.set_worm.is_(True),
                                           AntiRansomwarePolicyTable.need_detect.is_(False))
                                     | AntiRansomwarePolicyTable.id.is_(None))
    return sub_query


def convert_conditions(conditions) -> dict:
    filters = ("name", 'location', 'policy_name', "tenant_name", "detection_status")
    conditions = {k: v
                  for k, v in json.loads(conditions).items()
                  if k in filters} if conditions else {}
    return conditions


def query_resource_filter(conditions, sub_query):
    name = conditions.get('name', '')
    if name:
        sub_query = sub_query.filter(CopyTable.resource_name.ilike(f'%{name}%', escape='#'))
    location = conditions.get('location', '')
    if location:
        sub_query = sub_query.filter(CopyTable.resource_location.ilike(f'%{location}%', escape='#'))
    policy_name = conditions.get('policy_name', '')
    if policy_name:
        sub_query = sub_query.filter(AntiRansomwarePolicyTable.policy_name.ilike(f'%{policy_name}%', escape='#'))
    return sub_query


def query_resource_filter_by_detection_status(copies_subquery, query, detection_status_list):
    support_detection_num = [
        "uninspected_copy_num", "prepare_copy_num", "detecting_copy_num",
        "uninfected_copy_num", "infected_copy_num", "abnormal_copy_num"
    ]
    filters = []
    for detection_status in detection_status_list:
        if detection_status not in support_detection_num:
            raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS,
                                       message=f"detection_status (" + detection_status + ") illegal.")
        if detection_status == "uninfected_copy_num":
            filters.append(copies_subquery.c.uninfected_copy_num > 0)
        if detection_status == "infected_copy_num":
            filters.append(copies_subquery.c.infected_copy_num > 0)
        if detection_status == "abnormal_copy_num":
            filters.append(copies_subquery.c.abnormal_copy_num > 0)
        if detection_status == "prepare_copy_num":
            filters.append(copies_subquery.c.prepare_copy_num > 0)
        if detection_status == "detecting_copy_num":
            filters.append(copies_subquery.c.detecting_copy_num > 0)
        if detection_status == "uninspected_copy_num":
            filters.append(copies_subquery.c.uninspected_copy_num > 0)
    if filters:
        query = query.filter(or_(*filters))
    return query


def query_anti_ransomware_copies_resource_data(session, subquery, page_query_param: dict, conditions, orders: str):
    copies_subquery = get_copy_sub_query(session)

    query = session.query(CopyTable.resource_id, CopyTable.resource_name, CopyTable.resource_location,
                          subquery.c.policy_name, subquery.c.policy_id,
                          copies_subquery.c.total_copy_num,
                          copies_subquery.c.uninspected_copy_num,
                          copies_subquery.c.prepare_copy_num,
                          copies_subquery.c.detecting_copy_num,
                          copies_subquery.c.uninfected_copy_num,
                          copies_subquery.c.infected_copy_num,
                          copies_subquery.c.abnormal_copy_num,
                          subquery.c.tenant_name,
                          subquery.c.tenant_id,
                          copies_subquery.c.latest_detection_time,
                          CopyTable.resource_sub_type) \
        .join(copies_subquery, copies_subquery.c.uuid == CopyTable.uuid) \
        .join(subquery, subquery.c.resource_id == CopyTable.resource_id)
    if conditions.get("detection_status", None):
        query = query_resource_filter_by_detection_status(copies_subquery, query, conditions.get("detection_status"))
    count = query.count()
    # 排序分页
    if orders:
        if orders.startswith("+"):
            query = query.order_by(asc(orders[1:]))
        else:
            query = query.order_by(desc(orders[1:]))
    return query.limit(page_query_param.get("page_size")).offset(
        page_query_param.get("page_no") * page_query_param.get("page_size")).all(), count


def get_copy_sub_query(session):
    # 作为基础子查询
    copies_subquery = session.query(func.max(CopyTable.uuid).label("uuid"), CopyTable.resource_id,
                                    func.count(CopyTable.uuid).label("total_copy_num"),
                                    func.sum(case([(CopyAntiRansomwareTable.status.is_(None), 1),
                                        (CopyAntiRansomwareTable.status == AntiRansomwareEnum.UNDETECTED.value, 1)],
                                             else_=0)).label("uninspected_copy_num"),
                                    func.sum(
                                        case([(CopyAntiRansomwareTable.status == AntiRansomwareEnum.PREPARE.value, 1)],
                                             else_=0)).label("prepare_copy_num"),
                                    func.sum(case(
                                        [(CopyAntiRansomwareTable.status == AntiRansomwareEnum.DETECTING.value, 1)],
                                        else_=0)).label("detecting_copy_num"),
                                    func.sum(case(
                                        [(CopyAntiRansomwareTable.status == AntiRansomwareEnum.UNINFECTING.value, 1)],
                                        else_=0)).label("uninfected_copy_num"),
                                    func.sum(case(
                                        [(CopyAntiRansomwareTable.status == AntiRansomwareEnum.INFECTING.value, 1)],
                                        else_=0)).label("infected_copy_num"),
                                    func.sum(
                                        case([(CopyAntiRansomwareTable.status == AntiRansomwareEnum.ERROR.value, 1)],
                                             else_=0)).label("abnormal_copy_num"),
                                    func.max(CopyAntiRansomwareTable.detection_end_time).label("latest_detection_time")
                                    ) \
        .outerjoin(CopyAntiRansomwareTable, CopyAntiRansomwareTable.copy_id == CopyTable.uuid) \
        .filter(CopyTable.generated_by.in_(AntiRansomwareCopyConstant.SUPPORT_ANTI_COPY_GENERATED_BY_LIST)) \
        .filter(or_(CopyTable.resource_status == ResourceStatus.EXIST.value,
                CopyTable.generated_by.in_(AntiRansomwareCopyConstant.COPY_GENERATED_BY_REPLICATED_LIST))) \
        .group_by(CopyTable.resource_id).subquery()
    return copies_subquery


def query_latest_detection_time_correspond_tenant_info(resource_id):
    # 根据资源id查询，防勒索检测报告表中的最近一次检测时间且存在的租户信息
    with database.session() as session:
        resource_tenant_info = session.query(
            CopyTable.resource_id,
            CopyAntiRansomwareTable.tenant_name,
            CopyAntiRansomwareTable.tenant_id) \
            .join(CopyAntiRansomwareTable, CopyAntiRansomwareTable.copy_id == CopyTable.uuid) \
            .filter(CopyAntiRansomwareTable.tenant_id != "") \
            .filter(CopyTable.resource_id == resource_id) \
            .order_by(desc(CopyAntiRansomwareTable.detection_end_time)) \
            .first()
        if not resource_tenant_info:
            resource_tenant_info = ("", "", "")
        return resource_tenant_info


def convert_anti_ransomware_copies_resource_to_statistics(anti_ransomware_copies: List):
    items = []
    for result in anti_ransomware_copies:
        # 租户id 为空时，去查询的防勒索报告表信息。
        if result[13] in ["", None]:
            resource_tenant_info = query_latest_detection_time_correspond_tenant_info(result[0])
            tenant_name = resource_tenant_info[1]
            tenant_id = resource_tenant_info[2]
        else:
            tenant_name = result[12]
            tenant_id = result[13]
        # 更新资源名称，在底座文件系统修改的情况下会不一致
        name = query_latest_resource_name(result)
        copy_statistics = CopyAntiRansomwareStatistics(resource_id=result[0],
                                                       name=name,
                                                       location=result[2],
                                                       policy_name=result[3],
                                                       policy_id=result[4],
                                                       total_copy_num=result[5],
                                                       uninspected_copy_num=result[6],
                                                       prepare_copy_num=result[7],
                                                       detecting_copy_num=result[8],
                                                       uninfected_copy_num=result[9],
                                                       infected_copy_num=result[10],
                                                       abnormal_copy_num=result[11],
                                                       tenant_name=tenant_name,
                                                       tenant_id=tenant_id,
                                                       latest_detection_time=result[14],
                                                       resource_sub_type=result[15]
                                                       )
        items.append(copy_statistics)
    return items


def query_latest_resource_name(result):
    # 文件名更新后，以资源表中的名名称为准，
    with database.session() as session:
        resource_name = session.query(ResourceTable.name).filter(ResourceTable.uuid == result[0]).first()
        if resource_name and (result[1] != resource_name[0]):
            name = resource_name[0]
        else:
            name = result[1]
    return name


def query_anti_ransomware_copies_summary(token: str, resource_sub_type: List[str], period: str):
    check_deploy_type_support()
    user = get_user_info_from_token(token)
    with database.session() as session:
        sub_query = query_anti_ransomware_copies_resource_subquery(session, resource_sub_type, None, user)
        if period:
            copies_summary, start_date, end_date = query_anti_ransomware_copies_summary_data_by_period(
                session, sub_query.subquery(), period)
            return convert_anti_ransomware_copies_summary_by_period(copies_summary, start_date, end_date)
        else:
            copies_summary = query_anti_ransomware_copies_summary_data(session, sub_query.subquery())
            return convert_anti_ransomware_copies_summary(copies_summary)


def query_anti_ransomware_copies_summary_data(session, subquery):
    query = session.query(CopyTable.resource_sub_type,
                          func.count(CopyTable.uuid).label("total_copy_num"),
                          func.sum(case([(CopyAntiRansomwareTable.status.is_(None), 1),
                                         (CopyAntiRansomwareTable.status == AntiRansomwareEnum.UNDETECTED.value, 1)],
                                        else_=0)).label("uninspected_copy_num"),
                          func.sum(case([(CopyAntiRansomwareTable.status == AntiRansomwareEnum.PREPARE.value, 1)],
                                        else_=0)).label("prepare_copy_num"),
                          func.sum(case([(CopyAntiRansomwareTable.status == AntiRansomwareEnum.DETECTING.value, 1)],
                                        else_=0)).label("detecting_copy_num"),
                          func.sum(case([(CopyAntiRansomwareTable.status == AntiRansomwareEnum.UNINFECTING.value, 1)],
                                        else_=0)).label("uninfected_copy_num"),
                          func.sum(case([(CopyAntiRansomwareTable.status == AntiRansomwareEnum.INFECTING.value, 1)],
                                        else_=0)).label("infected_copy_num"),
                          func.sum(case([(CopyAntiRansomwareTable.status == AntiRansomwareEnum.ERROR.value, 1)],
                                        else_=0)).label("abnormal_copy_num")
                          ) \
        .join(subquery, subquery.c.resource_id == CopyTable.resource_id) \
        .outerjoin(CopyAntiRansomwareTable, CopyAntiRansomwareTable.copy_id == CopyTable.uuid) \
        .filter(CopyTable.generated_by.in_(AntiRansomwareCopyConstant.SUPPORT_ANTI_COPY_GENERATED_BY_LIST)) \
        .filter(or_(CopyTable.resource_status == ResourceStatus.EXIST.value,
                CopyTable.generated_by.in_(AntiRansomwareCopyConstant.COPY_GENERATED_BY_REPLICATED_LIST))) \
        .group_by(CopyTable.resource_sub_type)
    return query.all()


def query_anti_ransomware_copies_summary_data_by_period(session, subquery, period):
    # 取检测时间的日期部分，转为日期，2006-01-02 15:04:05 -> 2006-01-02
    detection_end_date = func.to_date(func.left(CopyAntiRansomwareTable.detection_end_time, 10), "YYYY-MM-DD")
    time_zone = None
    end_date = datetime.now(tz=time_zone).date()
    start_date = None
    if period == CopyDetectionResultPeriodEnum.WEEK.value:
        start_date = end_date - relativedelta(weeks=1)
    if period == CopyDetectionResultPeriodEnum.MONTH.value:
        start_date = end_date - relativedelta(months=1)
    if period == CopyDetectionResultPeriodEnum.HALF_YEAR.value:
        start_date = end_date - relativedelta(months=6)
    filters = [detection_end_date <= end_date]
    if start_date:
        filters.append(detection_end_date > start_date)
    query = session.query(detection_end_date,
                          func.count(CopyTable.uuid).label("total_copy_num"),
                          func.sum(case([(CopyAntiRansomwareTable.status.is_(None), 1),
                                         (CopyAntiRansomwareTable.status == AntiRansomwareEnum.UNDETECTED.value, 1)],
                                        else_=0)).label("uninspected_copy_num"),
                          func.sum(case([(CopyAntiRansomwareTable.status == AntiRansomwareEnum.PREPARE.value, 1)],
                                        else_=0)).label("prepare_copy_num"),
                          func.sum(case([(CopyAntiRansomwareTable.status == AntiRansomwareEnum.DETECTING.value, 1)],
                                        else_=0)).label("detecting_copy_num"),
                          func.sum(case([(CopyAntiRansomwareTable.status == AntiRansomwareEnum.UNINFECTING.value, 1)],
                                        else_=0)).label("uninfected_copy_num"),
                          func.sum(case([(CopyAntiRansomwareTable.status == AntiRansomwareEnum.INFECTING.value, 1)],
                                        else_=0)).label("infected_copy_num"),
                          func.sum(case([(CopyAntiRansomwareTable.status == AntiRansomwareEnum.ERROR.value, 1)],
                                        else_=0)).label("abnormal_copy_num")
                          ) \
        .join(subquery, subquery.c.resource_id == CopyTable.resource_id) \
        .outerjoin(CopyAntiRansomwareTable, CopyAntiRansomwareTable.copy_id == CopyTable.uuid) \
        .filter(and_(*filters),
                CopyTable.generated_by.in_(AntiRansomwareCopyConstant.SUPPORT_ANTI_COPY_GENERATED_BY_LIST)) \
        .filter(or_(CopyTable.resource_status == ResourceStatus.EXIST.value,
                CopyTable.generated_by.in_(AntiRansomwareCopyConstant.COPY_GENERATED_BY_REPLICATED_LIST))) \
        .group_by(detection_end_date) \
        .order_by(detection_end_date)
    return query.all(), start_date, end_date


def convert_anti_ransomware_copies_summary_by_period(anti_ransomware_copy_summary: List, start_date, end_date):
    """
    根据查询周期转换查询结果
    :param anti_ransomware_copy_summary: 查询结果
    :param start_date: 查询周期开始的时间
    :param end_date: 查询周期结束的时间
    :return: 按天存放的查询结果，没有结果的日期取默认值0
    """
    all_dates = []
    current_date = start_date + timedelta(days=1)
    while current_date <= end_date:
        all_dates.append(current_date)
        current_date += timedelta(days=1)

    query_result = {
        str(detect_date): CopyAntiRansomwareSummary(detection_date=str(detect_date),
                                                    total_copy_num=0,
                                                    uninspected_copy_num=0,
                                                    prepare_copy_num=0,
                                                    detecting_copy_num=0,
                                                    uninfected_copy_num=0,
                                                    infected_copy_num=0,
                                                    abnormal_copy_num=0) for detect_date in all_dates
    }
    for result in anti_ransomware_copy_summary:
        copy_anti_ransomware_summary = CopyAntiRansomwareSummary(detection_date=str(result[0]),
                                                                 total_copy_num=result[1],
                                                                 uninspected_copy_num=result[2],
                                                                 prepare_copy_num=result[3],
                                                                 detecting_copy_num=result[4],
                                                                 uninfected_copy_num=result[5],
                                                                 infected_copy_num=result[6],
                                                                 abnormal_copy_num=result[7])
        query_result[str(result[0])] = copy_anti_ransomware_summary
    return list(query_result.values())


def convert_anti_ransomware_copies_summary(anti_ransomware_copy_summary: List):
    items = []
    for result in anti_ransomware_copy_summary:
        copy_anti_ransomware_summary = CopyAntiRansomwareSummary(resource_sub_type=result[0],
                                                                 total_copy_num=result[1],
                                                                 uninspected_copy_num=result[2],
                                                                 prepare_copy_num=result[3],
                                                                 detecting_copy_num=result[4],
                                                                 uninfected_copy_num=result[5],
                                                                 infected_copy_num=result[6],
                                                                 abnormal_copy_num=result[7])
        items.append(copy_anti_ransomware_summary)
    return items


def query_anti_ransomware_copies(token, resource_id, page_no, page_size, orders, conditions):
    user = get_user_info_from_token(token)
    if page_size == 0:
        return BasePage()
    with database.session() as session:
        query = session.query(CopyTable, CopyAntiRansomwareTable) \
            .outerjoin(CopyAntiRansomwareTable, CopyTable.uuid == CopyAntiRansomwareTable.copy_id) \
            .filter(CopyTable.resource_id == resource_id) \
            .filter(CopyTable.generated_by.in_(AntiRansomwareCopyConstant.SUPPORT_ANTI_COPY_GENERATED_BY_LIST)) \
            .filter(or_(CopyTable.resource_status == ResourceStatus.EXIST.value,
                        CopyTable.generated_by.in_(AntiRansomwareCopyConstant.COPY_GENERATED_BY_REPLICATED_LIST)))
        if user.get('domain-id'):
            sub_query = session.query(DomainResourceObjectTable.resource_object_id).filter(
                DomainResourceObjectTable.domain_id == user.get('domain-id')).filter(
                DomainResourceObjectTable.type == ResourceSetTypeEnum.COPY).subquery()
            query = query.filter(CopyTable.uuid.in_(sub_query))

        query = query_anti_ransomware_copies_filter_by_conditions(conditions, query)

        # 获得总数量
        count = query.count()
        if count == 0:
            return BasePage(items=[], total=0, pages=0, page_no=page_no, page_size=0)

        order_dic = {
            "-display_timestamp": CopyTable.display_timestamp.desc(),
            "+display_timestamp": CopyTable.display_timestamp.asc(),
            "-detection_time": CopyAntiRansomwareTable.detection_start_time.desc(),
            "+detection_time": CopyAntiRansomwareTable.detection_start_time.asc()
        }
        # 查询每页数据
        results = query.order_by(order_dic.get(orders, CopyTable.display_timestamp.asc())).limit(page_size).offset(
            page_no * page_size).all()
        items = []
        for result in results:
            copy_anti_dict = {}
            if result[1]:
                copy_anti_dict.update(result[1].__dict__)
                copy_anti_dict['anti_status'] = result[1].status
                copy_anti_dict['detection_time'] = result[1].detection_start_time
            else:
                copy_anti_dict['anti_status'] = AntiRansomwareEnum.UNDETECTED.value
            copy_anti_dict.update(result[0].__dict__)
            items.append(CopyAntiRansomware(**copy_anti_dict))
        return BasePage(items=items, total=count, pages=math.ceil(count / page_size), page_no=page_no,
                        page_size=len(items))


def query_anti_ransomware_copies_filter_by_conditions(conditions, query):
    filters = ("anti_status", "detection_model_version", "uuid")
    conditions = {k: v for k, v in json.loads(conditions).items() if k in filters} if conditions else {}

    status = conditions.get('anti_status', [])
    if status:
        if AntiRansomwareEnum.UNDETECTED.value in status:
            # 副本未生成检测任务不会在CopyAntiRansomwareTable中生成数据
            query = query.filter(or_(CopyAntiRansomwareTable.status.__eq__(None),
                                     CopyAntiRansomwareTable.status.in_(status)))
        else:
            query = query.filter(CopyAntiRansomwareTable.status.in_(status))

    detection_model_version = conditions.get('detection_model_version', [])
    if detection_model_version:
        query = query.filter(CopyAntiRansomwareTable.model.in_(detection_model_version))

    uuid = conditions.get('uuid', "")
    if uuid:
        query = query.filter(CopyAntiRansomwareTable.copy_id.ilike(f'%{uuid}%', escape='#'))
    return query


def delete_copy_anti_ransomware_report(copy_id: str):
    with database.session() as session:
        report = session.query(CopyAntiRansomwareTable).filter(CopyAntiRansomwareTable.copy_id == copy_id).first()
        log.info(f"delete copy {copy_id} anti ransomware start.")
        if not report:
            log.info(f'Copy {copy_id} is not anti ransomware')
            return
    copies = [copy_id]
    try:
        delete_report(copies)
    except Exception:
        log.error(f"delete copy_id:{copy_id} anti ransomware report error.")
    finally:
        pass


def get_deleting_anti_ransomware_report(copy_id: str):
    with database.session() as session:
        report = session.query(CopyAntiRansomwareTable). \
            filter(CopyAntiRansomwareTable.copy_id == copy_id).one_or_none()
        report_schemas = convert_report_to_report_schemas(report)
    return report_schemas


def convert_report_to_report_schemas(report):
    report_schema = None
    if report:
        return CopyAntiRansomwareReportSchemas(**report.__dict__)
    return report_schema


def query_anti_ransomware_detail(copy_id: str):
    with database.session() as session:
        copy = session.query(CopyTable).filter(CopyTable.uuid == copy_id).one_or_none()
        if copy is None:
            log.error(f"Query anti-ransomware info is error. The copy_id:{copy_id} is not exist.")
            raise EmeiStorBizException(CopyErrorCode.COPY_NOT_EXIST)
        anti_ransomware_report = session.query(CopyAntiRansomwareTable). \
            filter(CopyAntiRansomwareTable.copy_id == copy_id).first()
        if not anti_ransomware_report:
            return CopyAntiRansomwareReport(copy_id=copy_id,
                                            timestamp=copy.display_timestamp
                                            .strftime(CommonConstants.COMMON_DATE_FORMAT),
                                            status=AntiRansomwareEnum.UNDETECTED.value
                                            )
        return CopyAntiRansomwareReport(copy_id=copy_id,
                                        timestamp=copy.display_timestamp
                                        .strftime(CommonConstants.COMMON_DATE_FORMAT),
                                        model=anti_ransomware_report.model,
                                        status=anti_ransomware_report.status,
                                        detection_duration=anti_ransomware_report.detection_duration,
                                        detection_time=anti_ransomware_report.detection_start_time,
                                        report=anti_ransomware_report.report
                                        )


def get_copy_is_security_snap(copy: CopyTable) -> bool:
    copy_properties = json.loads(copy.properties)
    if DeployType().is_cyber_engine_deploy_type():
        copy_generated_by = copy.generated_by
        # 安全一体机形态共享路径恢复删除copies表中基于安全快照的克隆文件系统信息时直接返回非安全快照
        if copy_generated_by is not None and copy_generated_by == GenerationType.BY_LIVE_MOUNTE.value:
            log.info(f"copy generated by {copy_generated_by}, skip security snap check.")
            return False
        resource_id = copy.resource_id
        resource = ResourceClient.query_resource(resource_id=resource_id)
        # 如果资源/设备信息被删除, 则无法获取设备信息, 作为非安全快照处理, 让快照删除校验继续往下执行
        if resource is None or resource == {}:
            log.info(f"resource not exist, skip security snap check:")
            return True
        env_info = ResourceClient.query_resource(resource_id=resource.get("root_uuid"))
        if env_info is None or env_info == {}:
            log.info(f"resource environment info not exist, skip security snap check:")
            return False
        if env_info.get("link_status") == LinkStatusEnum.Offline.value:
            raise EmeiStorBizException(CommonErrorCodes.STORAGE_DEVICE_CONNECT_ERROR,
                                       message="Storage device status is offline.")
        security_snap_info = SystemBaseClient.query_remote_storage_fssnapshot(copy_properties.get("snapshotId"),
                                                                              copy_properties.get("tenantId"),
                                                                              env_info.get("uuid"))
    else:
        security_snap_info = SystemBaseClient.query_local_storage_fssnapshot(copy_properties.get("snapshotId"),
                                                                             copy_properties.get("tenantId"))
    # 是安全快照，并且在保护期内.
    return security_snap_info.get("isSecuritySnap", False) and security_snap_info.get("isInProtectionPeriod", False)


def get_cdp_copy_is_security_snap(copy: CopyTable) -> bool:
    copy_properties = json.loads(copy.properties)
    security_snap_info = SystemBaseClient.query_local_storage_cdp(copy_properties.get("snapshotId"),
                                                                         copy_properties.get("tenantId"))
    # 是安全快照，并且在保护期内.
    return security_snap_info.get("secureSnapEnabled", False) and security_snap_info.get("isInProtectionPeriod", False)


def check_vmware_copy_is_expire_security_snap(copy: CopyTable) -> bool:
    copy_properties = json.loads(copy.properties)
    file_system_name = copy_properties.get("vmware_metadata")["disk_info"][0]["DISKDEVICENAME"]
    snapshot_name = copy_properties.get("vmware_metadata")["disk_info"][0]["DISKSNAPSHOTDEVICENAME"]
    security_snap_info = SystemBaseClient.get_fs_snapshot_by_names(file_system_name, snapshot_name)
    # 是安全快照，并且在保护期内.
    return security_snap_info.get("isSecuritySnap", False) and security_snap_info.get("isInProtectionPeriod", False)


def check_deploy_type_support():
    deploy_type = DeployType()
    # x3000部署方式下,屏蔽相关接口
    if deploy_type.is_x3000_type():
        log.error(f"In the x3000 deployment mode, the current operation is not supported.")
        raise EmeiStorBizException(CommonErrorCodes.SYSTEM_ERROR)


def query_anti_ransomware_resources_cyber_engine(tuple_param):
    token, page_no, page_size, orders, conditions = tuple_param
    if page_size == 0:
        return BasePage(items=[], total=0, pages=0, page_no=page_no, page_size=0)
    # 验证排序参数
    check_anti_ransomware_copies_resource_orders(orders)
    user = get_user_info_from_token(token) if token is not None else None
    with database.session() as session:
        # 获得资源列表
        sub_query = query_anti_ransomware_copies_resource_subquery_cyber_engine(session, conditions, user)
        count = sub_query.count()
        if count == 0:
            return BasePage(items=[], total=0, pages=0, page_no=page_no, page_size=0)
        subquery = sub_query.subquery()
        # 根据资源列表获得检测信息
        copies_statistics_query = query_anti_ransomware_resources_data_cyber_engine(session, subquery, orders,
                                                                                    conditions)
        count = copies_statistics_query.count()
        copies_statistics = copies_statistics_query.limit(page_size).offset(page_no * page_size).all()

        res_count_info = session.query(CopyTable.resource_id,
                                       func.sum(case([(
                                           CopyTable.generated_by.in_(
                                               AntiRansomwareCopyConstant.SUPPORT_ANTI_COPY_GENERATED_BY_LIST), 1)],
                                           else_=0)).label("total_copy_num"),
                                       func.sum(case([(
                                           CopyAntiRansomwareTable.status == AntiRansomwareEnum.INFECTING.value, 1)],
                                           else_=0)).label("infected_copy_num"),
                                       func.sum(case([(CopyAntiRansomwareTable.handle_detect_infect, 1)],
                                                     else_=0)).label("handle_false_count"),
                                       func.sum(case([(
                                           CopyAntiRansomwareTable.status != AntiRansomwareEnum.UNDETECTED.value, 1)],
                                           else_=0)).label("total_detect_copy_num")) \
            .outerjoin(CopyAntiRansomwareTable, CopyAntiRansomwareTable.copy_id == CopyTable.uuid).group_by(
            CopyTable.resource_id).all()

    all_device_map = query_cyber_detect_devices([])

    res_count_map = {}
    for res_count in res_count_info:
        res_count_map.update({res_count[0]: res_count})

    items = [convert_resource_db_model_2_gui_model(result, all_device_map, res_count_map) for
             result in copies_statistics]
    return BasePage(
        items=items,
        total=count,
        pages=math.ceil(count / page_size),
        page_no=page_no,
        page_size=len(items))


def query_anti_ransomware_copies_resource_subquery_cyber_engine(session, conditions, user):
    filters = ("resource_name", 'device_name', "tenant_name", "resource_id")
    convert_condition_dict = {k: v for k, v in json.loads(conditions).items()
                              if k in filters} if conditions else {}
    # 获得已绑定防勒索策略资源列表
    resource_list = session.query(CopyTable.resource_id,
                                  func.max(case([(ResExtendInfoTable.key == 'tenantName', ResExtendInfoTable.value)],
                                                else_="")).label('tenant_name'),
                                  func.max(case([(ResExtendInfoTable.key == 'tenantId', ResExtendInfoTable.value)],
                                                else_="")).label('tenant_id'),
                                  func.max(case([(ResExtendInfoTable.key == 'fileSubType', ResExtendInfoTable.value)],
                                                else_="")).label('file_sub_type'),
                                  func.max(ResourceTable.root_uuid).label('root_uuid')) \
        .join(ResExtendInfoTable, CopyTable.resource_id == ResExtendInfoTable.resource_id) \
        .join(ResourceTable, CopyTable.resource_id == ResourceTable.uuid) \
        .filter(CopyTable.generated_by.in_(AntiRansomwareCopyConstant.SUPPORT_ANTI_COPY_GENERATED_BY_LIST)) \
        .filter(or_(CopyTable.resource_status == ResourceStatus.EXIST.value,
                    CopyTable.generated_by.in_(AntiRansomwareCopyConstant.COPY_GENERATED_BY_REPLICATED_LIST)))
    if user is not None:
        if not check_user_is_admin_or_audit(user):
            resource_list = resource_list.filter(CopyTable.user_id == get_user_id_from_user_info(user))
    # 条件过滤
    resource_id = convert_condition_dict.get('resource_id', '')
    if resource_id:
        resource_list = resource_list.filter(CopyTable.resource_id == resource_id)
    resource_name = convert_condition_dict.get('resource_name', '')
    if resource_name:
        resource_list = resource_list.filter(CopyTable.resource_name.ilike(f'%{resource_name}%', escape='#'))
    cyber_detect_devices_ids = query_cyber_detect_devices(convert_condition_dict.get('device_name', [])).keys()
    resource_list = resource_list.filter(ResourceTable.root_uuid.in_(cyber_detect_devices_ids))
    resource_sub = resource_list.group_by(CopyTable.resource_id).subquery()
    resource_with_tenant_list = session.query(resource_sub.c.resource_id, resource_sub.c.tenant_name,
                                              resource_sub.c.tenant_id, resource_sub.c.file_sub_type,
                                              resource_sub.c.root_uuid)
    tenant_name = convert_condition_dict.get('tenant_name', '')
    if tenant_name:
        resource_with_tenant_list = resource_with_tenant_list.filter(
            resource_sub.c.tenant_name.ilike(f'%{tenant_name}%', escape='#'))
    return resource_with_tenant_list


def query_cyber_detect_devices(filter_names: List[str]):
    query_conditions = {"type": "StorageEquipment", "subType": [["!="], "StorageOthers"], "detectType": [["in"], "1"]}
    if filter_names:
        name_conditions = [["in"]]
        name_conditions.extend(filter_names)
        query_conditions.update({"name": name_conditions})
    devices = ResourceClient.query_v2_resource_list(query_conditions)
    device_map = {}
    for device in devices:
        device_map.update({device.get('uuid'): device})
    return device_map


def query_anti_ransomware_resources_data_cyber_engine(session, subquery, orders: str, conditions):
    copies_subquery = session.query(CopyTable.resource_id,
                                    func.max(CopyTable.display_timestamp).label("end_copy_time"),
                                    func.min(CopyTable.display_timestamp).label("start_copy_time")) \
        .group_by(CopyTable.resource_id).subquery()

    # 作为基础子查询
    query = session.query(CopyTable.resource_id, CopyTable.resource_name, CopyTable.resource_location,
                          CopyTable.resource_environment_ip,
                          CopyTable.resource_environment_name,
                          subquery.c.tenant_name,
                          subquery.c.tenant_id,
                          ResourceAntiRansomwareTable.copy_generated_time,
                          ResourceAntiRansomwareTable.status,
                          ResourceAntiRansomwareTable.total_file_size,
                          ResourceAntiRansomwareTable.added_file_count,
                          ResourceAntiRansomwareTable.changed_file_count,
                          ResourceAntiRansomwareTable.deleted_file_count,
                          ResourceAntiRansomwareTable.copy_detected_time,
                          ResourceAntiRansomwareTable.copy_id,
                          copies_subquery.c.end_copy_time,
                          copies_subquery.c.start_copy_time,
                          subquery.c.file_sub_type,
                          subquery.c.root_uuid,
                          ResourceAntiRansomwareTable.infected_file_count,
                          ResourceAntiRansomwareTable.backup_software,
                          ResourceAntiRansomwareTable.generate_type) \
        .join(copies_subquery, copies_subquery.c.resource_id == CopyTable.resource_id) \
        .join(subquery, subquery.c.resource_id == CopyTable.resource_id) \
        .outerjoin(CopyAntiRansomwareTable, CopyAntiRansomwareTable.copy_id == CopyTable.uuid) \
        .outerjoin(ResourceAntiRansomwareTable, ResourceAntiRansomwareTable.resource_id == CopyTable.resource_id)

    # 条件过滤
    query = query_resource_detect_result_cyber_engine_filter(conditions, query).distinct(CopyTable.resource_id) \
        .order_by(CopyTable.resource_id)
    # 排序分页
    if orders:
        if orders.startswith("+"):
            query = query.order_by(asc(orders[1:]))
        else:
            query = query.order_by(desc(orders[1:]))
    return query


def query_resource_detect_result_cyber_engine_filter(conditions, query):
    filters = ("status", "start_time", "end_time", "resource_id", "generate_type")
    conditions = {k: v for k, v in json.loads(conditions).items()
                  if k in filters} if conditions else {}
    status = conditions.get('status', [])
    query = query.filter(CopyTable.generated_by.in_(AntiRansomwareCopyConstant.SUPPORT_ANTI_COPY_GENERATED_BY_LIST))
    if status:
        if AntiRansomwareEnum.UNDETECTED.value in status:
            query = query.filter(or_(ResourceAntiRansomwareTable.status.__eq__(None),
                                     ResourceAntiRansomwareTable.status.in_(status)))
        else:
            query = query.filter(ResourceAntiRansomwareTable.status.in_(status))
    resource_id = conditions.get('resource_id', '')
    if resource_id:
        query = query.filter(CopyTable.resource_id == resource_id)
    generate_type = conditions.get('generate_type', [])
    if generate_type:
        query = query.filter(ResourceAntiRansomwareTable.generate_type.in_(generate_type))
    # 时间过滤和分组
    query = query_copy_by_start_and_end_time(conditions, query)
    return query


def query_copy_by_start_and_end_time(conditions, query):
    start_time = conditions.get('start_time', '')
    if start_time:
        datetime_start_time = datetime.strptime(start_time, CommonConstants.COMMON_DATE_FORMAT_WITH_T)
        query = query.filter(CopyTable.display_timestamp >= datetime_start_time)
    end_time = conditions.get('end_time', '')
    if end_time:
        datetime_end_time = datetime.strptime(end_time, CommonConstants.COMMON_DATE_FORMAT_WITH_T)
        query = query.filter(CopyTable.display_timestamp <= datetime_end_time)
    return query


def query_copy_by_expiration_time(expired, query):
    time_zone = None
    now = datetime.now(tz=time_zone)
    if expired == "true":
        query = query.filter(CopyTable.expiration_time < now)
    else:
        query = query.filter(or_(CopyTable.expiration_time >= now, CopyTable.retention_type == 1))
    return query


def query_anti_ransomware_copies_cyber_engine(token, resource_id, page_no, page_size, orders, conditions):
    user = get_user_info_from_token(token) if token is not None else None
    if page_size == 0:
        return BasePage()
    with database.session() as session:
        resource_with_tenant = query_anti_ransomware_copies_resource_subquery_cyber_engine(session, conditions, user)
        resource_with_tenant_sub = resource_with_tenant.subquery()
        query = session.query(CopyTable, CopyAntiRansomwareTable, resource_with_tenant_sub.c.tenant_id,
                              resource_with_tenant_sub.c.tenant_name, resource_with_tenant_sub.c.file_sub_type,
                              ResourceTable.root_uuid) \
            .outerjoin(CopyAntiRansomwareTable, CopyTable.uuid == CopyAntiRansomwareTable.copy_id) \
            .join(resource_with_tenant_sub, CopyTable.resource_id == resource_with_tenant_sub.c.resource_id) \
            .join(ResourceTable, CopyTable.resource_id == ResourceTable.uuid) \
            .filter(CopyTable.generated_by.in_(AntiRansomwareCopyConstant.SUPPORT_ANTI_COPY_GENERATED_BY_LIST)) \
            .filter(or_(CopyTable.resource_status == ResourceStatus.EXIST.value,
                        CopyTable.generated_by.in_(AntiRansomwareCopyConstant.COPY_GENERATED_BY_REPLICATED_LIST)))
        if resource_id is not None:
            query = query.filter(CopyTable.resource_id == resource_id)
        if user is not None:
            if not check_user_is_admin_or_audit(user):
                query = query.filter(CopyTable.user_id == get_user_id_from_user_info(user))
        # 过滤
        query = query_copy_detect_result_cyber_engine_filter(conditions, query, resource_with_tenant_sub)
        # 获得总数量
        count = query.count()
        if count == 0:
            return BasePage(items=[], total=0, pages=0, page_no=page_no, page_size=0)
        # 支持排序条件为: 快照生成时间/侦测结束时间、总文件大小、新增/修改/删除文件数量
        order_dic = get_query_anti_ransomware_copies_cyber_engine_order_dic()
        # 查询每页数据
        results = query.order_by(order_dic.get(orders)).limit(page_size).offset(page_no * page_size).all()
        # 更新设备信息
        all_device_map = query_cyber_detect_devices([])
        items = query_anti_ransomware_copies_cyber_engine_post_progress(all_device_map, results)
        return BasePage(
            items=items,
            total=count,
            pages=math.ceil(count / page_size),
            page_no=page_no,
            page_size=len(items))


def query_anti_ransomware_copies_cyber_engine_post_progress(all_device_map, results):
    # 展示数据后处理
    items = []
    for result in results:
        copy_anti_dict = {}
        if result[1]:
            copy_anti_dict.update(result[1].__dict__)
            copy_anti_dict['anti_status'] = result[1].status
            copy_anti_dict['detection_time'] = result[1].detection_end_time
        else:
            copy_anti_dict['anti_status'] = AntiRansomwareEnum.UNDETECTED.value
        copy_anti_dict.update(result[0].__dict__)
        copy_anti_dict.update({'tenant_id': result[2]})
        copy_anti_dict.update({'tenant_name': result[3]})
        copy_anti_dict.update({'file_sub_type': result[4]})
        copy_anti_dict.update({'resource_environment_name': all_device_map[result[5]]['name']})
        copy_anti_dict.update({'resource_environment_ip': all_device_map[result[5]]['endpoint']})
        items.append(convert_copy_db_model_2_gui_model(copy_anti_dict))
    return items


def get_query_anti_ransomware_copies_cyber_engine_order_dic():
    return {"-display_timestamp": CopyTable.display_timestamp.desc(),
            "+display_timestamp": CopyTable.display_timestamp.asc(),
            "-total_file_size": CopyAntiRansomwareTable.total_file_size.desc(),
            "+total_file_size": CopyAntiRansomwareTable.total_file_size.asc(),
            "-added_file_count": CopyAntiRansomwareTable.added_file_count.desc(),
            "+added_file_count": CopyAntiRansomwareTable.added_file_count.asc(),
            "-changed_file_count": CopyAntiRansomwareTable.changed_file_count.desc(),
            "+changed_file_count": CopyAntiRansomwareTable.changed_file_count.asc(),
            "-deleted_file_count": CopyAntiRansomwareTable.deleted_file_count.desc(),
            "+deleted_file_count": CopyAntiRansomwareTable.deleted_file_count.asc(),
            "-detection_time": CopyAntiRansomwareTable.detection_end_time.desc(),
            "+detection_time": CopyAntiRansomwareTable.detection_end_time.asc(),
            }


def query_copy_detect_result_cyber_engine_filter(conditions, query, resource_with_tenant):
    # 支持过滤条件为: 检测状态/副本状态/生成方式/设备名称列表, 快照名称/资源名称/租户名称模糊匹配,
    # 精确匹配: 单天的时间/副本ID, 范围匹配: 精确到秒的起始和结束时间
    filters = (
        'copy_status', 'anti_status', 'generate_type', 'resource_name', 'resource_environment_name', 'name',
        'tenant_name', 'date', 'start_time', 'end_time', 'uuid', 'expired'
    )
    conditions = {k: v for k, v in json.loads(conditions).items()
                  if k in filters} if conditions else {}
    anti_status = conditions.get('anti_status', [])
    copy_status = conditions.get('copy_status', [])
    query = query_copy_by_status_cyber_engine(anti_status, copy_status, query)
    uuid = conditions.get('uuid', '')
    if uuid:
        query = query.filter(CopyTable.uuid == uuid)
    generate_type = conditions.get('generate_type', [])
    if generate_type:
        query = query.filter(CopyAntiRansomwareTable.generate_type.in_(generate_type))
    resource_name = conditions.get("resource_name", '')
    if resource_name:
        query = query.filter(CopyTable.resource_name.ilike(f'%{resource_name}%', escape='#'))
    cyber_detect_devices_ids = query_cyber_detect_devices(conditions.get('resource_environment_name', [])).keys()
    query = query.filter(ResourceTable.root_uuid.in_(cyber_detect_devices_ids))
    name = conditions.get('name', '')
    if name:
        query = query.filter(CopyTable.name.ilike(f'%{name}%', escape='#'))
    tenant_name = conditions.get("tenant_name", '')
    if tenant_name:
        query = query.filter(resource_with_tenant.c.tenant_name.ilike(f'%{tenant_name}%', escape='#'))
    # 时间点(天)过滤
    date = conditions.get('date', '')
    if date:
        query = query.filter(func.to_char(CopyTable.display_timestamp, "YYYY-MM-DD") == date)
    # 时间区间过滤
    query = query_copy_by_start_and_end_time(conditions, query)
    # 过滤是否过期快照
    expired = conditions.get('expired', '')
    if expired:
        query = query_copy_by_expiration_time(expired, query)
    return query


def query_copy_by_status_cyber_engine(anti_status, copy_status, query):
    # 支持两种状态同时过滤，或的关系、支持单种状态过滤
    status_filter = None
    # 由于页面优先展示副本状态：恢复中、删除中、删除失败
    # 故防勒索检测状态过滤时，需要去掉副本状态为上述的三个状态
    if anti_status:
        if AntiRansomwareEnum.UNDETECTED.value in anti_status:
            status_filter = and_(or_(CopyAntiRansomwareTable.status.__eq__(None),
                                     CopyAntiRansomwareTable.status.in_(anti_status)),
                                 CopyTable.status.not_in(CYBER_ENGINE_SPECIAL_COPY_STATUS))
        else:
            status_filter = and_(or_(CopyAntiRansomwareTable.status.in_(anti_status)),
                                 CopyTable.status.not_in(CYBER_ENGINE_SPECIAL_COPY_STATUS))
    if copy_status:
        if status_filter is not None:
            status_filter = or_(status_filter, CopyTable.status.in_(copy_status))
        else:
            status_filter = CopyTable.status.in_(copy_status)
    if status_filter is not None:
        query = query.filter(status_filter)
    return query


def convert_copy_db_model_2_gui_model(copy_anti_dict):
    copy_gui_model = CopyAntiRansomware(**copy_anti_dict)
    copy_gui_model.anti_status = copy_anti_dict['anti_status']
    copy_gui_model.generate_type = copy_anti_dict.get('generate_type', None)
    copy_gui_model.is_security_snapshot = (copy_anti_dict['worm_status'] == 3)
    copy_gui_model.total_file_size = copy_anti_dict.get('total_file_size', None)
    copy_gui_model.changed_file_count = copy_anti_dict.get('changed_file_count', None)
    copy_gui_model.added_file_count = copy_anti_dict.get('added_file_count', None)
    copy_gui_model.deleted_file_count = copy_anti_dict.get('deleted_file_count', None)
    copy_gui_model.detection_time = copy_anti_dict.get('detection_time', None)
    copy_gui_model.tenant_name = copy_anti_dict.get('tenant_name', None)
    copy_gui_model.model = copy_anti_dict.get('model', None)
    copy_gui_model.handle_false = copy_anti_dict.get('handle_detect_infect', None)
    copy_gui_model.snapshot_time = format_datetime_2_gui_str(copy_anti_dict.get('display_timestamp'))
    copy_gui_model.file_sub_type = copy_anti_dict.get('file_sub_type', None)
    copy_gui_model.infected_file_count = copy_anti_dict.get('infected_file_count', None)
    copy_gui_model.backup_software = copy_anti_dict.get('backup_software', None)
    return copy_gui_model


def convert_resource_db_model_2_gui_model(result, all_devices, res_count_map):
    # 更新资源名称，在底座文件系统修改的情况下会不一致
    name = query_latest_resource_name(result)
    return CopyAntiRansomwareStatistics(
        resource_id=result[0],
        name=name,
        location=result[2],
        device_ip=all_devices[result[18]]['endpoint'],
        device_name=all_devices[result[18]]['name'],
        tenant_name=result[5],
        tenant_id=result[6],
        total_copy_num=res_count_map[result[0]][1],
        latest_snapshot_time=result[7],
        status=-1 if result[8] is None else result[8],
        total_file_size=result[9],
        added_file_count=result[10],
        changed_file_count=result[11],
        deleted_file_count=result[12],
        latest_detection_time=result[13],
        latest_copy_id=result[14],
        infected_copy_num=res_count_map[result[0]][2],
        handle_false_count=res_count_map[result[0]][3],
        end_copy_time=format_datetime_2_gui_str(result[15]),
        start_copy_time=format_datetime_2_gui_str(result[16]),
        total_detect_copy_num=res_count_map[result[0]][4],
        file_sub_type=result[17],
        infected_file_count=result[19],
        backup_software=result[20],
        generate_type=result[21]
    )


def format_datetime_2_gui_str(datetime_object):
    return None if datetime_object is None else datetime_object.strftime(CommonConstants.COMMON_DATE_FORMAT)


def is_generate_type_io_detect(copy_id: str) -> bool:
    if DeployType().is_cyber_engine_deploy_type():
        with database.session() as session:
            copy_anti_ransomware = session.query(CopyAntiRansomwareTable).filter(
                CopyAntiRansomwareTable.copy_id == copy_id).one_or_none()
        if copy_anti_ransomware is not None and copy_anti_ransomware.generate_type == "IO_DETECT":
            return True
    return False
