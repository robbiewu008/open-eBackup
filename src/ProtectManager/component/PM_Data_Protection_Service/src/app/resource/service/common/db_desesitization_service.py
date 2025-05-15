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
from app.base.db_base import database
from app.common import toolkit
from app.common.enums import db_desesitization_enum
from app.common.enums.db_desesitization_enum import DesesitizationStatusEnum, IdentificationStatusEnum, \
    IdentificationStatusMap, DesesitizationStatusMap, JobStatusMap
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.event_messages.Reports.progress_rest import JobType
from app.common.event_messages.event import EventBase
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.logger import get_logger
from app.common.toolkit import JobMessage
from app.resource.db import db_desestitation_db_api
from app.resource.kafka.topics import DB_RESOURCE_DESESTIZATION_CREATE_JOB
from app.resource.models.resource_models import ResourceDesesitizationTable
from app.resource.schemas.db_desesitization_schema import DbDesestitationTaskSchema
from app.resource.service.common import resource_service, domain_resource_object_service

log = get_logger(__name__)


class CreateJobMessage(EventBase):
    def __init__(self, topic, params: dict, request_id=None, response_topic=''):
        if params:
            for k, v in params.items():
                setattr(self, k, v)
        request_id = request_id if request_id else str(uuid.uuid4())
        EventBase.__init__(self, request_id, topic, response_topic)


def desestitation_db(params: DbDesestitationTaskSchema):
    with database.session():
        resource_info = resource_service.query_resource_by_id(params.resource_id)
        if resource_info is None:
            raise EmeiStorBizException(ResourceErrorCodes.HOST_NOT_EXISTS,
                                       message=f"Environment {params.resource_id} is not exists.")
        desesitization_info = db_desestitation_db_api.query_desesitization_info(params.resource_id)
        if desesitization_info is None:
            desesitization_info = ResourceDesesitizationTable()
            desesitization_info.uuid = params.resource_id
            desesitization_info.desesitization_status = DesesitizationStatusEnum.NotDesesitized
            desesitization_info.identification_status = IdentificationStatusEnum.NotIdentified
            desesitization_info.desesitization_policy_id = params.policy_id
            desesitization_info.desesitization_policy_name = params.policy_name
            db_desestitation_db_api.create_desesitization_info(desesitization_info)
        if params.task_type == db_desesitization_enum.TaskTypeEnum.ConfirmPolicy:
            desesitization_info.desesitization_policy_id = params.policy_id
            desesitization_info.desesitization_policy_name = params.policy_name
            db_desestitation_db_api.update_desesitization_info(desesitization_info)
        else:
            if params.task_status == db_desesitization_enum.TaskStatusEnum.Start:
                process_start_status(desesitization_info, params, resource_info)
            else:
                process_update_status(desesitization_info, params)


def process_start_status(desesitization_info, params, resource_info):
    environment_resource = resource_service.query_resource_by_id(resource_info.root_uuid)
    request_id = str(uuid.uuid4())
    if environment_resource is None:
        raise EmeiStorBizException(ResourceErrorCodes.HOST_NOT_EXISTS,
                                   message=f"Environment {resource_info.root_uuid} is not exists.")
    if params.task_type == db_desesitization_enum.TaskTypeEnum.Identification:
        desesitization_info.identification_status = IdentificationStatusMap.get(params.task_status, "")
        # 开始识别，需要把脱敏状态置为未脱敏
        desesitization_info.desesitization_status = DesesitizationStatusEnum.NotDesesitized
        job_message = JobMessage(
            topic=DB_RESOURCE_DESESTIZATION_CREATE_JOB,
            payload=params,
        )
        domain_id_list = domain_resource_object_service.get_domain_id_list(resource_info.uuid)
        job_id = toolkit.create_job_center_task(request_id, {
            'requestId': request_id,
            'userId': environment_resource.user_id,
            'domainIdList': domain_id_list,
            'sourceId': resource_info.uuid,
            'sourceLocation': resource_info.path,
            'sourceName': resource_info.name,
            'sourceType': resource_info.type,
            'sourceSubType': resource_info.sub_type,
            "type": JobType.DB_IDENTIFICATION.value
        }, job_message)
        desesitization_info.identification_job_id = job_id
    else:
        desesitization_info.desesitization_status = DesesitizationStatusMap.get(params.task_status, "")
        job_message = JobMessage(
            topic=DB_RESOURCE_DESESTIZATION_CREATE_JOB,
            payload=params,
        )
        domain_id_list = domain_resource_object_service.get_domain_id_list(resource_info.uuid)
        job_id = toolkit.create_job_center_task(request_id, {
            'requestId': request_id,
            'userId': environment_resource.user_id,
            'domainIdList': domain_id_list,
            'sourceId': resource_info.uuid,
            'sourceLocation': resource_info.path,
            'sourceName': resource_info.name,
            'sourceType': resource_info.type,
            'sourceSubType': resource_info.sub_type,
            "type": JobType.DB_DESESITIZATION.value
        }, job_message)
        desesitization_info.desesitization_job_id = job_id
    db_desestitation_db_api.update_desesitization_info(desesitization_info)


def process_update_status(desesitization_info, params):
    if params.task_type == db_desesitization_enum.TaskTypeEnum.Identification:
        desesitization_info.identification_status = IdentificationStatusMap.get(params.task_status, "")
        update_job(desesitization_info.identification_job_id, params)
    else:
        desesitization_info.desesitization_status = DesesitizationStatusMap.get(params.task_status, "")
        update_job(desesitization_info.desesitization_job_id, params)
    db_desestitation_db_api.update_desesitization_info(desesitization_info)


def update_job(job_id, params):
    job_log_info = {
        "jobLogs": [{
            "jobId": job_id,
            "startTime": params.log_time,
            "logInfo": params.task_log,
            "logInfoParam": params.task_log_params,
            "logDetail": params.task_log_detail,
            "logDetailParam": params.task_log_detail_params,
            "level": params.task_log_level
        }],
        "status": JobStatusMap.get(params.task_status, ""),
        "progress": params.task_progress
    }
    toolkit.modify_task_log(None, job_id, job_log_info)


def check_can_delete(resource_id: str):
    desesitization_info = db_desestitation_db_api.query_desesitization_info(resource_id)
    if desesitization_info is not None:
        if desesitization_info.desesitization_status == DesesitizationStatusEnum.Desesitizing or \
                desesitization_info.identification_status == IdentificationStatusEnum.Identifing:
            return False
    return True


def add_desestitation_info(resource_id: str):
    with database.session():
        resource_info = resource_service.query_resource_by_id(resource_id)
        if resource_info is None:
            log.warn(f"Environment {resource_id} is not exists.")
            return
        if resource_info.sub_type == ResourceSubTypeEnum.MySQL or \
                resource_info.sub_type == ResourceSubTypeEnum.Oracle or \
                resource_info.sub_type == ResourceSubTypeEnum.SQLServer:
            desesitization_info = db_desestitation_db_api.query_desesitization_info(resource_id)
            if desesitization_info is None:
                desesitization_info = ResourceDesesitizationTable()
                desesitization_info.uuid = resource_id
                desesitization_info.desesitization_status = DesesitizationStatusEnum.NotDesesitized
                desesitization_info.identification_status = IdentificationStatusEnum.NotIdentified
                db_desestitation_db_api.create_desesitization_info(desesitization_info)
            else:
                desesitization_info.desesitization_status = DesesitizationStatusEnum.NotDesesitized
                desesitization_info.identification_status = IdentificationStatusEnum.NotIdentified
                db_desestitation_db_api.update_desesitization_info(desesitization_info)


def clean_desestitation_info(resource_id: str):
    with database.session():
        desesitization_info = db_desestitation_db_api.query_desesitization_info(resource_id)
        if desesitization_info is None:
            return
        desesitization_info.desesitization_status = DesesitizationStatusEnum.NotDesesitized
        desesitization_info.identification_status = IdentificationStatusEnum.NotIdentified
        desesitization_info.desesitization_policy_id = None
        desesitization_info.desesitization_policy_name = None
        db_desestitation_db_api.update_desesitization_info(desesitization_info)
