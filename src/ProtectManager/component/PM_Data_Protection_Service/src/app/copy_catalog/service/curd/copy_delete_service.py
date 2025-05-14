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
from http import HTTPStatus
from http.client import HTTPException
from typing import List

from app.archive.schemas.archive_request import StorageProtocolEnum
from app.common import logger
from app.common.clients.client_util import SystemBaseHttpsClient, is_response_status_ok, parse_response_data
from app.common.clients.job_center_client import batch_query_job_list
from app.common.clients.scheduler_client import SchedulerClient
from app.common.deploy_type import DeployType
from app.common.enums.anti_ransomware_enum import AntiRansomwareEnum
from app.common.enums.copy_enum import GenerationType
from app.common.enums.job_enum import JobStatus, JobType
from app.common.enums.rbac_enum import ResourceSetTypeEnum
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import BackupTypeEnum
from app.common.event_messages.event import CommonEvent
from app.common.events import producer
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.redis_session import redis_session
from app.common.toolkit import complete_job_center_task, query_job_list
from app.copy_catalog.client.alarm_client import send_copy_expire_failed_alarm
from app.copy_catalog.common.common import ResourceStatus, CopyConstants
from app.copy_catalog.common.copy_status import CopyStatus
from app.copy_catalog.common.lock_service import LockService
from app.copy_catalog.models.tables_and_sessions import CopyTable, database, CopyProtectionTable
from app.copy_catalog.schemas import CopyArchiveMapSchema
from app.copy_catalog.schemas.copy_schemas import CopyAntiRansomwareQuery
from app.copy_catalog.service import anti_ransomware_service
from app.copy_catalog.service.anti_ransomware_service import delete_copy_anti_ransomware_report, \
    query_anti_ransomware_detail, refresh_resource_detect_table_with_lock
from app.copy_catalog.service.curd.copy_create_service import delete_copy_archive_map
from app.copy_catalog.service.curd.copy_query_service import get_copy
from app.copy_catalog.util.copy_util import check_copy_can_be_deleted
from app.job.job_util import get_job_by_id
from app.resource.service.common.domain_resource_object_service import delete_domain_resource_object_relation
from app.resource.service.common.resource_set_resource_service import delete_resource_set_resource_relation

log = logger.get_logger(__name__)
NAME = 'copy_data_catalog'


def check_copy_exist_clone_file_system(copy_id):
    with database.session() as session:
        copy: CopyTable = session.query(CopyTable.uuid).filter(CopyTable.parent_copy_uuid == copy_id). \
            filter(CopyTable.generated_by == GenerationType.BY_LIVE_MOUNTE.value).first()
        return copy is not None


def handle_copy_deleted(request_id: str):
    log.info(f"Handling copy deleted request_id: {request_id}")
    copy_id = redis_session.hget(request_id, "copy_id")
    job_id = redis_session.hget(request_id, "job_id")
    job_status = redis_session.hget(request_id, "job_status")

    if not job_id:
        log.exception("job_id not found in redis")
        return

    with database.session() as session:
        if not job_status:
            job = get_job_by_id(session, job_id)
            if not job:
                log.exception(f"Failed to query job with job_id {job_id}")
                return
            job_status = job.status

        status: JobStatus = JobStatus[job_status]

        query = session.query(CopyTable).filter(CopyTable.uuid == copy_id)
        copy: CopyTable = query.one_or_none()
        if copy is None:
            if not status:
                log.info(
                    f"copy isn't found, status doest not exist: request_id:{request_id} lock_id:{job_id} "
                    f"copy_id:{copy_id}.")
                return
            log.info(
                f"copy isn't found, release copy delete lock: request_id:{request_id} lock_id:{job_id} "
                f"copy_id:{copy_id}.")
        elif status == JobStatus.SUCCESS or status == JobStatus.PARTIAL_SUCCESS:
            update_copy_info_when_delete_copy_success(copy, session)
            update_copy_protection_info_when_delete_copy_success(copy, session)
        else:
            update_copy_info_when_delete_copy_fail(copy, request_id, session)
    response_topic = redis_session.hget(request_id, 'response_topic')
    if response_topic is None:
        complete_job_center_task(request_id, job_id, {
            "status": job_status,
            "progress": 100
        })
        redis_session.delete(request_id)
    else:
        response_param = redis_session.hget(request_id, 'response_param')
        param = json.loads(response_param)
        param['status'] = job_status
        message = CommonEvent(response_topic, request_id, **param)
        producer.produce(message)
    LockService.unlock_resources(request_id, job_id)
    log.info(f"release copy delete lock: request_id:{request_id} lock_id:{job_id}")


def update_copy_info_when_delete_copy_success(copy: CopyTable, session):
    prev_copy_id = copy.prev_copy_id
    next_copy_id = copy.next_copy_id
    copy_id = copy.uuid
    if prev_copy_id is not None:
        session.query(CopyTable).filter(CopyTable.uuid == prev_copy_id).update(
            {CopyTable.next_copy_id: next_copy_id})
    if next_copy_id is not None:
        session.query(CopyTable).filter(CopyTable.uuid == next_copy_id).update(
            {CopyTable.prev_copy_id: prev_copy_id})
    properties = json.loads(copy.properties)
    if copy.deletable:
        delete_copy_anti_ransomware_report(copy_id)
        log.info(f'Deleting copy from Database,copy_id:{copy_id}')
        session.query(CopyTable).filter(CopyTable.uuid == copy_id).delete()
        clean_archive_copy_map(copy, properties)
        update_copy_domain_and_resource_set_relation(copy_id)
        log.info(f'Refreshing resource after delete copy_id:{copy_id}')
        refresh_resource_detect_table_with_lock(copy.resource_id)
        log.info(f"copy(id={copy_id}) is deleted")
    else:
        session.query(CopyTable).filter(CopyTable.uuid == copy_id).update(
            {CopyTable.deleted: True})
        full_copy = session.query(CopyTable).filter(
            CopyTable.backup_type == BackupTypeEnum.full.value,
            CopyTable.generated_by == copy.generated_by,
            CopyTable.chain_id == copy.chain_id,
            CopyTable.gn < copy.gn).order_by(CopyTable.gn.desc()).first()
        if full_copy is None:
            properties["full_copy_id"] = ""
        else:
            properties["full_copy_id"] = full_copy.uuid
        session.query(CopyTable).filter(CopyTable.uuid == copy_id).update(
            {CopyTable.properties: json.dumps(properties)})
        log.info(f"copy(id={copy_id}) is signed as deleted")


def clean_archive_copy_map(copy, properties):
    if copy.is_archived:
        storage_id = ''
        repositories = properties.get('repositories', [])
        for rep in repositories:
            if rep.get('protocol', 5) in {StorageProtocolEnum.S3.value, StorageProtocolEnum.TAPE.value}:
                storage_id = rep.get('id', '')
        param = CopyArchiveMapSchema(copy_id=copy.parent_copy_uuid, storage_id=storage_id,
                                     resource_id=copy.resource_id)
        log.info(f"Start to delete archive map info, parent copy:{copy.parent_copy_uuid}, storage_id:{storage_id},"
                 f"resource_id:{copy.resource_id}")
        delete_copy_archive_map(param)


def update_copy_protection_info_when_delete_copy_success(copy: CopyTable, session):
    count = session.query(CopyTable).filter(CopyTable.resource_id == copy.resource_id) \
        .filter(CopyTable.deleted.is_(False)).count()
    if count == 0:
        # 取消被删除副本的自动保护
        copy_protected_object = session.query(CopyProtectionTable)\
            .filter(CopyProtectionTable.protected_resource_id == copy.resource_id).one_or_none()
        if copy_protected_object is None:
            return
        schedule_ids = list(task.uuid for task in copy_protected_object.task_list)
        SchedulerClient.batch_delete_schedules(schedule_ids)
        # 删除副本保护对象
        session.delete(copy_protected_object)
        log.info(f'after delete copy, delete copy protected object. '
                 f'protected_resource_id: {copy_protected_object.protected_resource_id}, '
                 f'copy id:{copy.uuid}')
        session.flush()


def delete_single_copy(copy_id: str):
    with database.session() as session:
        copy: CopyTable = session.query(CopyTable).filter(CopyTable.uuid == copy_id).one_or_none()
        if copy is None:
            log.info(f"copy(id={copy_id}) is not exist")
            update_copy_domain_and_resource_set_relation(copy_id)
            return
        prev_copy_id = copy.prev_copy_id
        next_copy_id = copy.next_copy_id
        if prev_copy_id is not None:
            session.query(CopyTable).filter(CopyTable.uuid == prev_copy_id).update(
                {CopyTable.next_copy_id: next_copy_id})
        if next_copy_id is not None:
            session.query(CopyTable).filter(CopyTable.uuid == next_copy_id).update(
                {CopyTable.prev_copy_id: prev_copy_id})
        session.query(CopyTable).filter(CopyTable.uuid == copy_id).delete()
        update_copy_protection_info_when_delete_copy_success(copy, session)
        update_copy_domain_and_resource_set_relation(copy_id)
        log.info(f"copy(id={copy_id}) is deleted")


def update_copy_domain_and_resource_set_relation(copy_id=None):
    if not copy_id:
        return
    delete_domain_resource_object_relation(resource_object_id=copy_id, resource_type=ResourceSetTypeEnum.COPY.value)
    delete_resource_set_resource_relation(resource_object_id=copy_id, resource_type=ResourceSetTypeEnum.COPY.value)


def handle_resource_removed(request_id: str, resource_id: str):
    log.info(f"Handle resource removed. request: {request_id}")
    is_hyper_detect_deploy_type = DeployType().is_hyper_detect_deploy_type()
    with database.session() as session:
        if is_hyper_detect_deploy_type:
            # 本地文件系统删除后，快照都被删除了，pm同步删除副本数据
            session.query(CopyTable).filter(CopyTable.resource_id == resource_id).delete(synchronize_session=False)
        else:
            session.query(CopyTable).filter(CopyTable.resource_id == resource_id).update({
                CopyTable.resource_status: ResourceStatus.NOT_EXIST.value
            })


def delete_copy_list(copy_id_list: List[str]):
    log.info(f"Only delete copy: {copy_id_list} in database.")
    with database.session() as session:
        session.query(CopyTable).filter(CopyTable.uuid.in_(copy_id_list)).delete(synchronize_session=False)


def delete_copy_protection(resource_ids: List[str]):
    with database.session() as session:
        session.query(CopyProtectionTable).filter(CopyProtectionTable.protected_resource_id.in_(resource_ids)) \
            .delete(synchronize_session=False)


def delete_resource_copy_index(resource_id: str, user_id: str):
    try:
        response = SystemBaseHttpsClient().request("DELETE", f"/v1/internal/copies/index?resource_id={resource_id}"
                                                             f"&user_id={user_id}")
    except Exception as ex:
        raise HTTPException(HTTPStatus.INTERNAL_SERVER_ERROR, str(ex)) from ex
    finally:
        pass
    if not is_response_status_ok(response):
        log.error(f'Invoke delete resource copy index failed.')
        raise EmeiStorBizException.build_from_error(parse_response_data(response.data))
    log.info(f'delete resource id {resource_id} copy index success.')
    return resource_id


def delete_archive_copy_by_storage_id(storage_id):
    with database.session() as session:
        copy_id_list = session.query(CopyTable.uuid).filter(CopyTable.properties.like(f'%{storage_id}%')).all()
    copy_id_list = [tmp[0] for tmp in copy_id_list]
    delete_copy_list(copy_id_list)
    log.info(f"delete copy in storage({storage_id}) success")


def check_copy_can_delete(copy_id_list):
    """
    判断副本能否被删除
    :param copy_id_list: 副本id list
    :return: 判断结果
    """
    can_delete = True
    for copy_id in copy_id_list:
        copy = get_copy(copy_id, False)
        if not copy:
            continue
        # 判断副本能否被删除
        _, error_message = check_copy_can_be_deleted(
            check_copy_exist_clone_file_system(copy_id), copy)
        if error_message:
            log.warn(f"copy({copy_id}) can not be delete.")
            can_delete = False
            break
    return {"result": 0 if can_delete else -1}


def resource_has_copy_link(copy) -> bool:
    # 资源对应副本存在链路关系。后续新增需要在这里增加 对应资源的条件返回True，则执行副本链的删除或者过期
    if copy.resource_sub_type == ResourceSubTypeEnum.CloudBackupFileSystem.value \
            and copy.backup_type != BackupTypeEnum.snapshot.value:
        return True
    return False


def update_copy_info_when_delete_copy_fail(copy: CopyTable, request_id, session):
    copy_id = copy.uuid
    copy_damaged = redis_session.hget(request_id, "copy_damaged")
    if copy_damaged == "true":
        session.query(CopyTable).filter(CopyTable.uuid == copy_id).filter(
            CopyTable.status == CopyStatus.DELETING.value).update({CopyTable.status: CopyStatus.DELETEFAILED.value})
        log.info(f"copy(id={copy_id}) is changed status to DELETEFAILED")
    else:
        session.query(CopyTable).filter(CopyTable.uuid == copy_id).filter(
            CopyTable.status == CopyStatus.DELETING.value).update({CopyTable.status: CopyStatus.NORMAL.value})
        log.info(f"copy(id={copy_id}) is changed status to Normal")
    job_type = redis_session.hget(request_id, "job_type")
    # 副本过期失败
    if job_type == JobType.COPY_EXPIRE.value:
        log.error(f"copy expire job delete failed, copy_id: {copy_id}, request_id: {request_id}")
        # 副本状态更新为删除失败
        update_copy_status(copy_id, CopyStatus.DELETEFAILED)
        if check_copy_expire_fail_last_for_one_month(copy_id):
            log.error(f"copy :{copy_id} expire failed last for one month, change status to delete failed.")
            # 持续过期失败一个月，发送告警
            send_copy_expire_failed_alarm(copy.resource_sub_type, copy.resource_id, copy.resource_name, copy_id,
                                          request_id)


def check_copy_expire_fail_last_for_one_month(copy_id: str):
    response = query_job_list({
        'copyId': copy_id,
        'types': JobType.COPY_EXPIRE.value,
        'statusList': [JobStatus.FAIL.value],
        'pageSize': 1,
        'startPage': 1,
        "orderType": "asc",
        "orderBy": "start_time"
    })
    if response is None:
        log.info(f"query copy :{copy_id} expire job failed.")
        return False
    response_info = json.loads(response)
    total_count = response_info.get("totalCount")
    if total_count <= 0:
        # 这次任务之前不存在副本过期失败任务
        log.info(f"copy :{copy_id} not exist expire job.")
        return False
    copy_expire_job = response_info.get("records")[0]
    # expire_job_start_time 单位秒
    expire_job_start_time = copy_expire_job.get("startTime") / 1000
    timezone = None
    now = datetime.datetime.now(tz=timezone).timestamp()
    if now > expire_job_start_time + CopyConstants.MONTH_IN_SECOND:
        return True
    return False


def update_copy_status(copy_id: str, status: CopyStatus):
    with database.session() as session:
        session.query(CopyTable).filter(CopyTable.uuid == copy_id).update({CopyTable.status: status.value})


def check_expire_copy_reach_to_delete_time(copy: CopyTable):
    """
    检查副本过期是否到删除时间
    1. 状态为正常的副本，到了过期时间直接删除
    2. 状态为删除失败的副本，因为删除失败，再次删除也可能失败。延迟删除执行时间，分别为1h, 2h, 4h, 8h, 16h, 32h，最大为7天
    :param copy: 过期副本
    :return: 是否到删除时间
    """
    copy_id = copy.uuid
    if copy.status != CopyStatus.DELETEFAILED.value:
        return True
    response = query_job_list({
        'copyId': copy_id,
        'types': JobType.COPY_EXPIRE.value,
        'statusList': [JobStatus.FAIL.value],
        'pageSize': 2,
        'startPage': 1,
        "orderType": "desc",
        "orderBy": "start_time"
    })
    if response is None:
        log.info(f"query copy :{copy_id} expire job failed, use default expire copy interval time.")
        return True
    response_info = json.loads(response)
    total_count = response_info.get("totalCount")
    if total_count <= 0:
        # 这次任务之前不存在副本过期失败任务
        log.info(f"copy :{copy_id} not exist expire job, use default expire copy interval time.")
        return True
    timezone = None
    now = datetime.datetime.now(tz=timezone).timestamp()
    pre_copy_expire_job = response_info.get("records")[0]
    last_expire_time = pre_copy_expire_job.get("startTime") / 1000
    if total_count == 1:
        next_expire_time = last_expire_time + CopyConstants.TWO_HOUR_SECOND
        log.info(f"copy: {copy_id} last_expire_time: {last_expire_time}, next_expire_time: {next_expire_time}")
        return now >= next_expire_time
    pre_pre_copy_expire_job = response_info.get("records")[1]
    last_two_job_interval_time = last_expire_time - (pre_pre_copy_expire_job.get("startTime") / 1000)
    interval_time = min(last_two_job_interval_time * 2, CopyConstants.EXPIRE_JOB_MAX_INTERVAL_SECOND)
    next_expire_time = last_expire_time + interval_time
    return now >= next_expire_time


def check_copy_anti_ransomware(copy: CopyTable):
    """
    查询所有未感染且未在删除中的副本数量 如果是1 并且是当前副本 自动过期不执行
    :param copy: 副本
    :return: 是否可以过期删除
    """
    copy_anti_ransomware_report = query_anti_ransomware_detail(copy.uuid)
    if copy_anti_ransomware_report.status != AntiRansomwareEnum.UNINFECTING.value:
        return False
    # 查询当前资源的所有副本过期/删除任务，从任务列表取出副本ID列表
    params = {
        "sourceId": copy.resource_id,
        "statusList": [JobStatus.READY.value, JobStatus.PENDING.value, JobStatus.RUNNING.value,
                       JobStatus.ABORTING.value],
        "types": [JobType.COPY_DELETE.value, JobType.COPY_EXPIRE.value]
    }
    jobs = batch_query_job_list(params)
    deleting_copy_uuids = [job.get('copyId', '') for job in jobs]
    # 再查出当前资源所有未感染副本，取出副本ID列表 B
    uninfected_copy_uuids = batch_query_uninfected_copies(copy)
    # 做差集C=B-A。 如C还剩一个副本，且是当前副本，则自动过期不执行
    deleting_copy_set = set(deleting_copy_uuids)
    uninfected_not_deleting_uuids = []
    for uninfected_copy_uuid in uninfected_copy_uuids:
        if uninfected_copy_uuid not in deleting_copy_set:
            uninfected_not_deleting_uuids.append(uninfected_copy_uuid)
    return (len(uninfected_not_deleting_uuids) == 1) and (uninfected_not_deleting_uuids[0] == copy.uuid)


def batch_query_uninfected_copies(copy):
    """
    查询资源下所有未感染副本的uuid
    :param copy: 副本
    :return: uuid列表
    """
    uninfected_copy_uuids = []
    page_no = 0
    page_size = 200
    while page_no >= 0:
        copy_anti_ransoware_query = CopyAntiRansomwareQuery(**{
            "resource_id": copy.resource_id,
            "page_no": page_no,
            "page_size": page_size,
            "status": AntiRansomwareEnum.UNINFECTING.value
        })
        base_page = anti_ransomware_service.query_copy_anti_ransomware(copy_anti_ransoware_query)
        uninfected_copy_uuids.extend([result.uuid for result in base_page.items])
        if len(base_page.items) < page_size:
            page_no = -1
        else:
            page_no += 1
    return uninfected_copy_uuids
