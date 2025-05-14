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
from typing import List

from sqlalchemy import true
from sqlalchemy.orm import Query

from app.common import logger
from app.common.clients import job_center_client
from app.common.context.context import Context
from app.common.enums.job_enum import JobType, JobStatus, JobLogLevel
from app.common.event_messages.event import EventBase
from app.common.events import producer
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.redis_session import redis_session
from app.common.toolkit import complete_job_center_task, query_job_list
from app.copy_catalog.common.constant import JobLabel
from app.copy_catalog.common.copy_status import CopyStatus
from app.copy_catalog.common.lock_service import LockService
from app.copy_catalog.copy_error_code import CopyErrorCode
from app.copy_catalog.models.tables_and_sessions import CopyTable, database
from app.copy_catalog.service.cloud_backup_service import associated_deletion_copies
from app.copy_catalog.service.curd.copy_delete_service import delete_single_copy, check_copy_exist_clone_file_system, \
    resource_has_copy_link
from app.copy_catalog.service.curd.copy_query_service import get_deleting_copy, get_resource_latest_copy, \
    query_copy_by_id
from app.copy_catalog.service.curd.copy_update_service import update_copy_status
from app.copy_catalog.util.copy_util import check_copy_can_be_deleted, abort, check_exist_copy_job, \
    create_delete_copy_job, get_delete_copy_job_type, check_should_be_stop_by_job_status
from app.job.job_util import record_job_step, update_job
from app.resource_lock.service import lock_service

log = logger.get_logger(__name__)


class DeleteCopyRequest(EventBase):
    copy_id: str
    job_id: str

    def __init__(self, copy_id: str, request_id: str, job_id: str):
        EventBase.__init__(self, request_id, "copy.delete")
        self.copy_id = copy_id
        self.job_id = job_id


class CopyDeleteParam:
    """
    field is_visible: 是否显示
    """
    def __init__(self, user_id: str, strict: bool = False, request_id=None,
                 job_id=None, create_job=True, job_type=JobType.COPY_DELETE.value, is_forced=False,
                 is_associated=True, is_delete_data=True):
        self.user_id = user_id
        self.strict = strict
        self.request_id = request_id
        self.job_id = job_id
        self.create_job = create_job
        self.job_type = job_type
        self.is_forced = is_forced
        self.is_associated = is_associated
        self.is_delete_data = is_delete_data


def request_delete_copy_by_id(copy_id: str, copy_delete_param: CopyDeleteParam):
    """
    根据副本ID删除副本
    :param copy_id: 副本ID
    :param copy_delete_param: 删除副本参数
    :return:
    """
    delete_res = None
    copy = get_deleting_copy(copy_id, copy_delete_param.strict)
    if copy is None:
        return delete_res
    if resource_has_copy_link(copy):
        for associated_copy in associated_deletion_copies(copy, association_type="down"):
            # 删除副本，当前副本到下一个全量副本之间的副本都删除
            log.info(f"start delete associated copy uuid: {associated_copy.uuid}, gn: {associated_copy.gn}")
            request_delete_copy(associated_copy, copy_delete_param)
        return delete_res
    else:
        return request_delete_copy(copy, copy_delete_param)


def batch_delete_copy(copy_list, copy_delete_param: CopyDeleteParam):
    log.info(f"Start batch delete copy, copy num is {len(copy_list)}")
    for copy_id in copy_list:
        try:
            request_delete_copy_by_id(copy_id, copy_delete_param)
        except Exception as err:
            log.exception(f'Delete copy:{copy_id} fail, Err:{err}')


def request_delete_copy(copy: CopyTable, copy_delete_param: CopyDeleteParam):
    delete_res = None
    request_id = copy_delete_param.request_id if copy_delete_param.request_id else str(uuid.uuid4())
    copy_id = copy.uuid
    timestamp = copy.timestamp
    resource_name = copy.resource_name
    is_last_copy = "false"
    is_member_deleted = json.loads(copy.properties).get("isMemberDeleted", False)
    if is_member_deleted == 'true':
        log.info(f'Member already been deleted, only deleting copy record.copy id:{copy_id}')
        copy_delete_param.create_job = False
    # 不下发删除任务，只用删除数据库记录
    if not copy_delete_param.create_job:
        delete_single_copy(copy_id)
        return delete_res

    # 不满足删除条件，不下发删除任务
    error_code, error_message = check_copy_can_be_deleted(check_copy_exist_clone_file_system(copy_id), copy)
    if error_message:
        log.info(f"copy({copy_id}) can not be deleted. message is: {error_message}")
        return abort(error_code, error_message, copy_delete_param.strict)

    if copy.generated_by in ['Backup', 'CloudArchive', 'Replicated']:
        latest_copy = get_resource_latest_copy(copy.resource_id, copy.generated_by)
        if latest_copy is None:
            message = f"The copy(resource_name={resource_name},copy_id={copy_id},timestamp={timestamp}) is deleted."
            return abort(CommonErrorCodes.OBJ_NOT_EXIST, message, copy_delete_param.strict)
        if latest_copy == copy_id:
            is_last_copy = "true"

    job_type = get_delete_copy_job_type(copy.generated_by, copy_delete_param.job_type)

    running_job_status = [
        JobStatus.READY.value, JobStatus.PENDING.value, JobStatus.RUNNING.value,
        JobStatus.ABORTING.value, JobStatus.DISPATCHING.value, JobStatus.REDISPATCH.value
    ]
    if job_type == JobType.COPY_DELETE.value:
        check_exist_copy_delete_job(copy_id, running_job_status)
    else:
        if check_exist_copy_job(copy_id, [JobType.COPY_DELETE.value, JobType.COPY_EXPIRE.value], running_job_status):
            log.info(f"Copy {copy_id} exist copy delete job, cancel create copy delete job.")
            return delete_res

    create_delete_copy_job_param = build_delete_copy_job_param(copy, copy_delete_param, is_last_copy, job_type,
                                                               request_id)
    log.info(f"Start to create job({job_type}) for copy({copy_id})")
    return create_delete_copy_job(create_delete_copy_job_param, copy)


def check_exist_copy_delete_job(copy_id: str, job_status: List[str]):
    if check_exist_copy_job(copy_id, [JobType.COPY_EXPIRE.value], job_status):
        log.info(f"Copy {copy_id} exist copy expire job, cancel create copy delete job.")
        raise EmeiStorBizException(CopyErrorCode.EXIST_COPY_DELETE_JOB, JobLabel.EXPIRE_COPY_JOB_LABEL)
    if check_exist_copy_job(copy_id, [JobType.COPY_DELETE.value], job_status):
        log.info(f"Copy {copy_id} exist copy delete job, cancel create copy delete job.")
        raise EmeiStorBizException(CopyErrorCode.EXIST_COPY_DELETE_JOB, JobLabel.DELETE_COPY_JOB_LABEL)


def build_delete_copy_job_param(copy, copy_delete_param, is_last_copy, job_type, request_id):
    return {
        'jobId': copy_delete_param.job_id,
        'requestId': request_id,
        'type': job_type,
        'userId': copy_delete_param.user_id,
        "isVisible": get_job_is_visible(copy),
        "jobType": job_type,
        "isLastCopy": is_last_copy,
        "isForced": copy_delete_param.is_forced,
        "isAssociated": copy_delete_param.is_associated,
        "isDeleteData": copy_delete_param.is_delete_data
    }


def get_job_is_visible(copy):
    return "false" if copy.deleted else "true"


def process_copy_delete_context_init(
        request_id,
        copy_id,
        is_visible,
        is_last_copy,
        resource_id,
        **extend_params
):
    log.info(f"copy delete init. request id= {request_id}, copy id= {copy_id}")
    job_id = request_id
    if not job_center_client.query_is_job_present(job_id):
        return
    try:
        if check_should_be_stop_by_job_status(job_id):
            log.info(f"Copy delete job: {job_id} stopped when init, copy id: {copy_id}")
            return
    except EmeiStorBizException as _:
        record_job_step(job_id, request_id, 'task_running_failed_label', JobLogLevel.FATAL)
        update_job(job_id, request_id, JobStatus.FAIL, JobLogLevel.FATAL)
        return
    update_copy_status(copy_id, CopyStatus.DELETING)
    redis_session.hset(request_id, "copy_id", copy_id)
    redis_session.hset(request_id, "is_visible", is_visible)
    redis_session.hset(request_id, "is_last_copy", is_last_copy)
    redis_session.hset(request_id, "is_forced", str(extend_params.get('is_forced')))
    redis_session.hset(request_id, "is_associated", str(extend_params.get('is_associated')))
    redis_session.hset(request_id, "is_delete_data", str(extend_params.get('is_delete_data')))
    redis_session.hset(request_id, "user_id", str(extend_params.get('user_id')))
    redis_session.hset(request_id, "lock_id", job_id)
    LockService.copy_delete_lock_resources(request_id, resource_id, copy_id, job_id)


def handle_copy_delete_locked(request_id: str, error_desc: str, status: str):
    log.info(f"copy delete resource lock: request_id:{request_id}, status:{status}, error_desc:{error_desc}")
    context = Context(request_id)
    job_id = context.get("job_id")
    copy_id = context.get("copy_id")
    if status != "success":
        update_copy_status(copy_id, CopyStatus.NORMAL)
        complete_job_center_task(request_id, job_id, {
            "status": "FAIL",
            "progress": 100
        })
        log.warn(f"copy delete resource lock fail")
        return
    if is_task_finished(job_id):
        log.info(f"job is already finished, release lock :{job_id}")
        lock_service.unlock(request_id, job_id)
        restore_copy_status(copy_id)
        return

    log.info(f"copy delete resource lock success, request_id:{request_id} lock_id:{job_id}")
    job_type = context.get("job_type")
    log.info(f"after locked start to delete copy : job_id:{job_id} job_type:{job_type} copy_id:{copy_id}")
    message = DeleteCopyRequest(copy_id, request_id, job_id)
    producer.produce(message)


def restore_copy_status(copy_id):
    log.info(f"restore deleting copy status. copy id:{copy_id}")
    with database.session() as session:
        session.query(CopyTable).filter(CopyTable.uuid == copy_id).filter(
            CopyTable.status == CopyStatus.DELETING.value).update({CopyTable.status: CopyStatus.NORMAL.value})


def is_task_finished(job_id):
    response = query_job_list({
        'statusList': [JobStatus.SUCCESS.value, JobStatus.FAIL.value, JobStatus.ABORTED.value,
                       JobStatus.CANCELLED.value, JobStatus.PARTIAL_SUCCESS.value, JobStatus.ABORT_FAILED.value],
        'jobId': job_id
    })
    if response is None:
        log.warn(f"query job {job_id} failed.")
        return False
    response_info = json.loads(response)
    total_count = response_info.get("totalCount")
    return total_count > 0


def handle_protection_removed(request_id: str, resource_id: str):
    for generated_by in ['Backup', 'CloudArchive', 'Replicated']:
        latest_copy_id = get_resource_latest_copy(resource_id, generated_by)
        if latest_copy_id is None:
            return
        latest_copy: CopyTable = query_copy_by_id(latest_copy_id)
        if latest_copy is None:
            return
        delete_leftover_copy(request_id, latest_copy, generated_by)


def delete_leftover_copy(request_id: str, latest_copy: CopyTable, generated_by: str):
    with database.session() as session:
        resource_id = latest_copy.resource_id
        conditions = (
            CopyTable.resource_id == resource_id,
            CopyTable.gn < latest_copy.gn,
            CopyTable.deleted == true(),
            CopyTable.generated_by == generated_by
        )
        query: Query = session.query(CopyTable).filter(*conditions)
        leftover_copies: List[CopyTable] = query.all()
        for leftover_copy in leftover_copies:
            resource_name = leftover_copy.resource_name
            copy_id = leftover_copy.uuid
            log.info(f"delete leftover copy(resource_id={resource_id}, resource_name={resource_name}, \
copy_id={copy_id}). request_id={request_id}")
            copy_delete_param = CopyDeleteParam(user_id=leftover_copy.user_id)
            request_delete_copy(leftover_copy, copy_delete_param)
