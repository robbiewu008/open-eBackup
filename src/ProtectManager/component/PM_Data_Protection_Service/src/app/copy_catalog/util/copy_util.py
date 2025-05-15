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
import datetime
import json
import math
import re
from typing import Dict, Optional, List

from sqlalchemy.orm import Query

from app.common import logger
from app.common.clients.anti_ransomware_client import AntiRansomwareClient
from app.common.clients.copy_manager_client import CopyManagerClient
from app.common.clients.job_center_client import get_job_queue_scope
from app.common.clients.system_base_client import SystemBaseClient
from app.common.deploy_type import DeployType
from app.common.enums.anti_ransomware_enum import AntiRansomwareEnum
from app.common.enums.copy_enum import GenerationType, CopyFormatEnum
from app.common.enums.job_enum import JobStatus, JobType
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import BackupTypeEnum
from app.common.enums.sla_enum import RetentionTypeEnum, RetentionTimeUnit, add_time, time_interval_switcher
from app.common.exception.common_error_codes import CommonErrorCodes, BaseErrorCode
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.toolkit import create_job_center_task, JobMessage
from app.common.toolkit import query_job_list
from app.copy_catalog.client.dee_client import check_copy_browse_status
from app.copy_catalog.common import copy_topics
from app.copy_catalog.common.common import IndexStatus
from app.copy_catalog.common.copy_status import CopyStatus, CopyWormStatus
from app.copy_catalog.copy_error_code import CopyErrorCode
from app.copy_catalog.models.tables_and_sessions import database, CopyTable, SystemConfigEntity
from app.copy_catalog.schemas import CopySchema, CopyRetentionPolicySchema
from app.copy_catalog.service.anti_ransomware_service import get_deleting_anti_ransomware_report, \
    get_copy_is_security_snap, check_vmware_copy_is_expire_security_snap, get_cdp_copy_is_security_snap
from app.job.job_util import get_job_by_id_with_session
from app.resource.service.common import resource_service, domain_resource_object_service
from app.restore.client.copy_client import CopyClient
from app.restore.schema.restore import RestoreLocation, RestoreType

log = logger.get_logger(__name__)


class Pagination:
    def __init__(self, total: int, page: int, page_size: int, items: list):
        self.total = total  # 数据总量
        self.page = page  # 当前页码
        self.page_size = page_size  # 每页条数
        self.items = items  # 当前页数据

    @property
    def total_pages(self):
        return (self.total + self.page_size - 1) // self.page_size


# 分页查询函数
def paginate(
        query,
        page: int = 1,
        page_size: int = 10
) -> Pagination:
    """
    从数据库中分页查询数据。
    :param query: SQLAlchemy 查询对象
    :param page: 当前页码（从 1 开始）
    :param page_size: 每页条数
    :return: Pagination 对象
    """
    if page < 1:
        page = 1
    if page_size < 1:
        page_size = 10

    total = query.count()  # 获取总条目数
    offset = (page - 1) * page_size
    items = query.offset(offset).limit(page_size).all()  # 获取当前页数据

    return Pagination(total=total, page=page, page_size=page_size, items=items)


def check_copy_can_be_deleted(exist_sub_copy, cur_copy):
    """
    检查副本是否可以被删除
    :param exist_sub_copy: 存在子副本
    :param cur_copy: 待删除副本
    :return: 不能删除返回对应的error_code和error_message
    """
    error_message = ""
    error_code = ""
    copy_status = CopyStatus.get(cur_copy.status)
    copy_id = cur_copy.uuid
    timestamp = cur_copy.timestamp
    resource_name = cur_copy.resource_name

    # 在细粒度浏览状态下的副本存在克隆文件系统，不允许删除
    if not DeployType().is_not_support_dee_restful_deploy_type() and check_copy_browse_status(cur_copy.uuid):
        error_message = f"copy exist clone file system, copy_id{copy_id}, timestamp={timestamp}"
        error_code = CopyErrorCode.ERROR_DELETE_BROWSING_COPY
    # 副本有克隆文件系统不下发删除
    if exist_sub_copy:
        error_message = f"copy exist clone file system, copy_id{copy_id}, timestamp={timestamp}"
        error_code = CopyErrorCode.ERROR_DELETE_COPY_EXIST_CLONE_FILE_SYSTEM
    if copy_status in [CopyStatus.RESTORING, CopyStatus.MOUNTING, CopyStatus.UNMOUNTING, CopyStatus.DELETING]:
        error_message = f"The copy(resource_name={resource_name},copy_id={copy_id},\
timestamp={timestamp}) is {copy_status.lower()}."
        error_code = CommonErrorCodes.STATUS_ERROR
    # 副本索引状态为Indexing时不允许删除
    if cur_copy.indexed == IndexStatus.INDEXING.value:
        error_message = f"The copy(resource_name={resource_name},copy_id={copy_id},timestamp={timestamp}) is indexing"
        error_code = CopyErrorCode.FORBID_DELETE_INDEXING_COPY
    # 副本在防勒索检测时，不允许删除
    anti_ransomware_report = get_deleting_anti_ransomware_report(copy_id)
    if anti_ransomware_report and anti_ransomware_report.status == AntiRansomwareEnum.DETECTING:
        raise_copy_or_snapshot_is_detecting_exception()
    # 除了即时挂载副本和归档副本，其余类型副本防勒索快照是安全快照不允许删除。
    if is_snapshot_in_period(cur_copy) and not is_generated_type_can_delete(cur_copy):
        raise EmeiStorBizException(CopyErrorCode.COPY_IS_SECURITY_SNAP)
    # 判断关联副本能否被删除
    check_associated_copies_can_be_deleted(cur_copy)
    return error_code, error_message


def is_generated_type_can_delete(cur_copy):
    return cur_copy.generated_by == GenerationType.BY_LIVE_MOUNTE.value \
        or cur_copy.generated_by == GenerationType.BY_TAPE_ARCHIVE.value \
        or cur_copy.generated_by == GenerationType.BY_CLOUD_ARCHIVE.value


def is_snapshot_in_period(cur_copy):
    if cur_copy.resource_sub_type == ResourceSubTypeEnum.CloudBackupFileSystem.value \
            and cur_copy.backup_type == BackupTypeEnum.snapshot.value \
            and get_copy_is_security_snap(cur_copy):
        return True
    if cur_copy.resource_sub_type == ResourceSubTypeEnum.LUN.value \
            and cur_copy.backup_type == BackupTypeEnum.snapshot.value \
            and get_cdp_copy_is_security_snap(cur_copy):
        return True
    if check_is_snapshot_copy_in_ocean_protect(cur_copy):
        log.info(f"Copy: {cur_copy.uuid} is a security snapshot, can not be deleted now.")
        return True
    return False


def raise_copy_or_snapshot_is_detecting_exception():
    if DeployType().is_cyber_engine_deploy_type():
        raise EmeiStorBizException(CopyErrorCode.SNAPSHOT_IS_DETECTING)
    else:
        raise EmeiStorBizException(CopyErrorCode.COPY_IS_DETECTING)


def check_is_snapshot_copy_in_ocean_protect(cur_copy: CopyTable) -> bool:
    """
    检查op的副本是否是安全快照
    """
    deploy_type = DeployType()
    if not deploy_type.is_ocean_protect_type():
        return False
    try:
        if cur_copy.resource_sub_type == ResourceSubTypeEnum.VirtualMachine.value:
            log.info(f"Copy: {cur_copy.uuid} is a VirtualMachine")
            return check_vmware_copy_is_expire_security_snap(cur_copy)
        if is_snapshot_worm(cur_copy):
            copy_properties = json.loads(cur_copy.properties)
            security_snap_info = SystemBaseClient.query_local_storage_fssnapshot(
                copy_properties.get("snapshots")[0].get("id"),
                copy_properties.get("tenantId", ""))
            return security_snap_info.get("isSecuritySnap", False) and security_snap_info.get("isInProtectionPeriod",
                                                                                              False)
        if is_directory_worm(cur_copy):
            # 目录格式副本的WORM策略未到期时，不允许删除
            log.info(f"Copy:{cur_copy.uuid} is a directory")
            return not AntiRansomwareClient.get_copy_expire_status(cur_copy)
        return False
    except Exception as ex:
        log.error(f"check copy({cur_copy.uuid}) is security snapshot occurs error : {ex}")
        return False


def is_directory_worm(copy):
    copy_format = json.loads(copy.properties).get("format")
    log.debug(f"Copy: {copy.uuid} format: {copy_format}")
    return copy_format == CopyFormatEnum.INNER_DIRECTORY.value and not is_not_exist_worm_copy(copy.as_dict())


def is_snapshot_worm(copy):
    copy_format = json.loads(copy.properties).get("format")
    log.debug(f"Copy: {copy.uuid} format: {copy_format}")
    if copy_format is None:
        return True
    return copy_format == CopyFormatEnum.INNER_SNAPSHOT.value or copy_format == CopyFormatEnum.EXTERNAL.value


def check_associated_copies_can_be_deleted(cur_copy):
    """
    判断该副本的关联能否被删除，如果关联副本不能删除，则该副本也不能被删除。

    Args:
        cur_copy: 当前副本

    Returns: 无

    """
    related_copies = set(CopyManagerClient.query_associated_copies(cur_copy.uuid))
    if cur_copy.uuid in related_copies:
        related_copies.remove(cur_copy.uuid)
    if len(related_copies) == 0:
        return
    with database.session() as session:
        condition = {
            CopyTable.uuid.in_(related_copies)
        }
        page = 1
        page_size = 10
        while True:
            query = session.query(CopyTable).filter(*condition)
            pagination = paginate(query, page, page_size)
            check_snapshot_in_period(cur_copy, pagination)
            if page >= pagination.total_pages:
                break
            page += 1


def check_snapshot_in_period(cur_copy, pagination):
    for item in pagination.items:
        if is_snapshot_in_period(item) and not is_generated_type_can_delete(item):
            log.error(f"The related copy({item.uuid} of copy({cur_copy.uuid}) is worm.")
            raise EmeiStorBizException(CopyErrorCode.DELETE_WORM_RELATED_COPY_FAIL, item.uuid)


def check_exist_copy_job(copy_id: str, types: List[str], job_status: List[str], extend_field: str = None):
    query_req = {
        'copyId': copy_id,
        'types': types,
        'statusList': job_status
    }
    if extend_field:
        query_req["extendStr"] = extend_field
    response = query_job_list(query_req)

    if response is None:
        return False

    # 如果发现已经存在任务，则不会创建任务
    response_info = json.loads(response)
    total_count = response_info.get("totalCount")
    if total_count > 0:
        log.warn(
            f"copy {types} task already exists, copy id is:{copy_id},totalCount:{total_count}")
        return True
    return False


def check_copy_has_supported_stopping(copy: CopyTable) -> bool:
    # 部分不支持副本删除任务和过期任务的类型
    copy_not_support_delete = [ResourceSubTypeEnum.CloudBackupFileSystem.value, ResourceSubTypeEnum.LUN.value]
    return copy.resource_sub_type not in copy_not_support_delete


def build_job_payload(payload: dict, copy: CopyTable, job_type: str):
    if job_type == JobType.COPY_EXPIRE.value:
        log.debug(f"Copy expire no need to set queue scope. copy id: {copy.uuid}")
        return payload
    resource_id = copy.resource_id
    resource_obj = resource_service.query_resource_by_id(resource_id)
    root_uuid = None
    if resource_obj:
        root_uuid = resource_obj.root_uuid
    queue_scope = get_job_queue_scope(copy.resource_sub_type, JobType.COPY_DELETE.value)
    if queue_scope:
        payload["queue_job_type"] = JobType.COPY_DELETE.value
        payload[queue_scope] = root_uuid
    return payload


def create_delete_copy_job(params: Dict, copy: CopyTable):
    job_extend_field = {'source_copy_type': copy.source_copy_type}
    domain_id_list = domain_resource_object_service.get_domain_id_list(copy.uuid)
    create_job_center_task(
        params.get('requestId'),
        {
            'jobId': params.get('jobId'),
            'sourceId': copy.resource_id,
            'sourceLocation': copy.resource_location,
            'sourceName': copy.resource_name,
            'sourceType': copy.resource_type,
            'sourceSubType': copy.resource_sub_type,
            'type': params.get('jobType'),
            'userId': params.get('userId'),
            'domainIdList': domain_id_list,
            'copyId': copy.uuid,
            "isVisible": params.get('isVisible'),
            'copyTime': math.floor(copy.display_timestamp.timestamp() * 1000),
            'enableStop': check_copy_has_supported_stopping(copy),
            'extendField': job_extend_field
        },
        message=JobMessage(
            topic=copy_topics.CopyTopic.COPY_DELETE_INIT,
            payload=build_job_payload({
                "copy_id": copy.uuid,
                "is_visible": params.get('isVisible'),
                "is_last_copy": params.get('isLastCopy'),
                "resource_id": copy.resource_id,
                "is_forced": params.get('isForced'),
                "is_associated": params.get('isAssociated'),
                "is_delete_data": params.get('isDeleteData'),
                "user_id": params.get('userId')
            }, copy, params.get('jobType'))
        )
    )


def check_copy_name_not_include_invalid_character(copy_name: str):
    # 校验副本名是否有效
    if not copy_name:
        return
    pattern = "^[a-zA-Z_\\u4e00-\\u9fa5]{1}[\\u4e00-\\u9fa5\\w-]*$"
    if re.match(pattern, copy_name) is None:
        log.error(f"Copy name: {copy_name} is invalid.")
        raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                   message=f"copy name [{copy_name}] is invalid.")


def get_copy_default_name(resource_name: str, copy_timestamp: str):
    return resource_name + "_" + str(int(int(copy_timestamp) / 1000000))


def get_latest_full_copy(query: Query):
    copy_in_db = None
    result = query.filter(CopyTable.backup_type == BackupTypeEnum.full.value).first()
    if result:
        # 获取最新的全量副本
        properties = json.loads(result.properties)
        if properties.get("backup_type") == "full":
            return CopySchema.from_orm(result)
    return copy_in_db


def get_present_full_copy(copy: dict):
    with database.session() as session:
        query = session.query(CopyTable).filter(CopyTable.resource_id == copy.get("resource_id", ""))
        query = query.filter(CopyTable.gn <= copy.get("gn"))
        query = query.filter(CopyTable.backup_type == BackupTypeEnum.full.value)
        query = query.filter(CopyTable.generated_by == GenerationType.BY_BACKUP.value)
        copy = query.order_by(CopyTable.timestamp.desc()).first()
    return copy if copy else None


def get_boolean_value_from_config_by_key(key: str):
    with database.session() as session:
        result = session.query(SystemConfigEntity.value).filter(SystemConfigEntity.key == key).scalar()
        log.info(f"Result is:{result}")
        return 'true' == result


def is_archive_copy_exist(present_full_copy: CopyTable):
    log.info(f"Start to check Full copy:{present_full_copy.uuid} is exist.")
    with database.session() as session:
        copy_obj = session.query(CopyTable).filter(CopyTable.parent_copy_uuid == present_full_copy.uuid,
                                                   CopyTable.generated_by.in_([GenerationType.BY_TAPE_ARCHIVE.value,
                                                                               GenerationType.BY_CLOUD_ARCHIVE.value]))\
            .first()
        return True if copy_obj else False


def pick(data: dict, mapping: dict, default=None):
    result = {}
    for k, v in mapping.items():
        result[v] = data[k] if k in data else default
    return result


def compare_copy_expiration_time(copy: CopySchema, modify_copy: CopyTable,
                                 retention_policy: CopyRetentionPolicySchema):
    if copy is None or modify_copy is None:
        return True
    log.info(f"[copy] copy uuid: {copy.uuid}, modify copy uuid: {modify_copy.uuid}.")
    # 如果保留策略是永久保留
    # 1. 对于全量副本， 返回true
    # 2. 对于增量副本，如果其依赖的全量副本如果是永久保留，返回true，否则，返回false
    if retention_policy.retention_type == RetentionTypeEnum.permanent.value:
        if modify_copy.backup_type == BackupTypeEnum.full.value:
            return True
        else:
            return copy.retention_type == RetentionTypeEnum.permanent.value

    # 如果所对应的全量/增量副本是永久保留的
    # 1. 所对应的是全量副本，不管增量副本过期时间如何改，都没事，返回true
    # 2. 所对应的是增量副本，全量副本是定点过期的，返回false
    if copy.retention_type == RetentionTypeEnum.permanent.value:
        return copy.backup_type == BackupTypeEnum.full.value

    copy_expired_time = copy.expiration_time
    modify_copy_expired_time = add_time(modify_copy.display_timestamp,
                                        retention_policy.retention_duration, retention_policy.duration_unit)
    log.info(f"[copy] copy expired time: {copy_expired_time}, modify copy expired time: {modify_copy_expired_time}.")
    if copy.backup_type == BackupTypeEnum.full.value:
        return copy_expired_time >= modify_copy_expired_time
    else:
        return modify_copy_expired_time >= copy_expired_time


def get_expiration_time(retention_policy: CopyRetentionPolicySchema, timestamp):
    temporary = retention_policy.retention_type == RetentionTypeEnum.temporary
    if temporary:
        expiration_time = add_time(timestamp, retention_policy.retention_duration, retention_policy.duration_unit)
    else:
        expiration_time = None
    return expiration_time


def get_valid_retention_info(retention_policy: CopyRetentionPolicySchema, timestamp,
                             generated_by=GenerationType.BY_BACKUP):
    worm_validity_type = retention_policy.worm_validity_type
    temporary = retention_policy.retention_type == RetentionTypeEnum.temporary
    duration_unit: Optional[str]
    if temporary:
        limits = {
            RetentionTimeUnit.days: 25550,
            RetentionTimeUnit.weeks: 3650,
            RetentionTimeUnit.months: 840,
            RetentionTimeUnit.years: 70,
        } if generated_by in [GenerationType.BY_TAPE_ARCHIVE.value, GenerationType.BY_CLOUD_ARCHIVE.value,
                              GenerationType.BY_CASCADED_REPLICATION.value, GenerationType.BY_REPLICATED.value,
                              GenerationType.BY_REVERSE_REPLICATION.value] else {
            RetentionTimeUnit.days: 365,
            RetentionTimeUnit.weeks: 54,
            RetentionTimeUnit.months: 24,
            RetentionTimeUnit.years: 10,
        }
        limit = limits.get(retention_policy.duration_unit)
        if retention_policy.retention_duration > limit:
            message = f"retention duration exceeds the maximum value({limit})"
            raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, error_message=message)
        if retention_policy.retention_duration < 1:
            message = f"retention duration exceeds the minimum value(1)"
            raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, error_message=message)
        duration_unit = retention_policy.duration_unit.value
        unit = time_interval_switcher.get(duration_unit)(1)
        if unit == 0:
            message = f"retention duration_unit get wrong value zero"
            raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, error_message=message)
        diff = datetime.datetime.now() - timestamp
        retention_duration = math.floor(diff.total_seconds() / unit)
        # 保留时间不能小于设置sla的最小的保留时间
        if retention_policy.retention_duration <= retention_duration:
            message = f"retention duration exceeds the minimum value({retention_duration})"
            raise_update_copy_or_snapshot_expire_time_exception(message)
        expiration_time = add_time(timestamp, retention_policy.retention_duration, retention_policy.duration_unit)
        retention_duration = retention_policy.retention_duration
    else:
        duration_unit = None
        retention_duration = 0
        expiration_time = None
    return duration_unit, expiration_time, retention_duration, temporary, worm_validity_type


def raise_update_copy_or_snapshot_expire_time_exception(message):
    if DeployType().is_cyber_engine_deploy_type():
        raise EmeiStorBizException(CopyErrorCode.ERROR_UPDATE_SNAPSHOT_EXPIRE_TIME, error_message=message)
    else:
        raise EmeiStorBizException(CopyErrorCode.ERROR_UPDATE_COPY_EXPIRE_TIME, error_message=message)


def get_future_time(**kwargs):
    return datetime.datetime.now() + datetime.timedelta(**kwargs)


def check_copy_status(copy_id: str, status: str, strict: bool):
    forbid_status = [
        CopyStatus.DELETING, CopyStatus.RESTORING, CopyStatus.MOUNTING, CopyStatus.MOUNTED, CopyStatus.SHARING,
        CopyStatus.DOWNLOADING
    ]
    valid = status not in list(item.value for item in forbid_status)
    check_status(valid, strict, CommonErrorCodes.STATUS_ERROR, f"The copy {copy_id} is {status.lower()}.")


def check_status(normal: bool, strict: bool, code: BaseErrorCode, message: str):
    if not normal:
        abort(code, message, strict)


def abort(code: BaseErrorCode, message: str, strict: bool):
    if strict:
        raise EmeiStorBizException(code, message=message)
    else:
        log.error(message)


def get_delete_copy_job_type(generation_type, default_job_type):
    if generation_type == GenerationType.BY_TAPE_ARCHIVE.value:
        return JobType.COPY_EXPIRE.value
    return default_job_type


def is_worm_copy(copy: dict):
    return copy.get("worm_status") not in [CopyWormStatus.UNSET.value, CopyWormStatus.EXPIRED.value]


# 是否不是worm转换的副本
def is_not_exist_worm_copy(copy: dict):
    return copy.get("worm_status") == CopyWormStatus.UNSET.value


def check_should_be_stop_by_job_status(job_id: str):
    job = get_job_by_id_with_session(job_id)
    if job is None:
        log.exception(f"Failed to query job with job_id {job_id}")
        raise EmeiStorBizException(CommonErrorCodes.SYSTEM_ERROR,
                                   message=f"Failed to query job with job_id {job_id}")
    log.info(f"Check whether should stop job by status, job_id:{job_id},status:{job.status}")
    return job.status not in [JobStatus.RUNNING.value, JobStatus.PENDING.value, JobStatus.READY.value]


def check_copy_operation_with_restore_limit(restore_req):
    copy_id = restore_req.copy_id
    if restore_req.restore_type == RestoreType.FLR.value:
        operation = 'FLR_RESTORE'
    elif restore_req.restore_type == RestoreType.IR.value:
        operation = 'LIVE_RESTORE'
    elif restore_req.restore_location == RestoreLocation.new.value:
        operation = 'NEW_LOCATION_RESTORE'
    else:
        operation = 'OLD_LOCATION_RESTORE'

    res_map = CopyClient().query_copy_operation_limit([copy_id], operation)
    if res_map and res_map.get(copy_id) is False:
        raise EmeiStorBizException(error=CommonErrorCodes.INFECTED_COPY_CAN_NOT_OPERATION,
                                   message="Copy can not do the operation because of infected.")
