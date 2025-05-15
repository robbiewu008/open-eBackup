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
from datetime import datetime, timezone

from app.backup.client.job_client import JobClient
from app.base.db_base import database
from app.common import logger
from app.common.constants.constant import ReplicationConstants
from app.common.enums.copy_enum import CopyFormatEnum
from app.common.enums.job_enum import JobType
from app.common.enums.schedule_enum import ExecuteType
from app.common.enums.sla_enum import PolicyActionEnum, PolicyTypeEnum, BackupTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.toolkit import JobMessage
from app.copy_catalog.service.curd.copy_query_service import query_copy_by_id, get_same_chain_copies
from app.replication.client.replication_client import ReplicationClient
from app.replication.models.rep_models import RepSlaUser
from app.replication.schemas.replication_request import ReplicationRequest
from app.resource.service.common.domain_resource_object_service import get_domain_id_list
from app.resource.service.common.resource_service import query_target_cluster_by_id
from app.common.rpc.system_base_rpc import encrypt
from app.common.util.cleaner import clear

log = logger.get_logger(__name__)


def reverse_replicate(user_id: str, replication_req: ReplicationRequest):
    """
    执行反向复制

    为副本创建反向复制任务；反向复制会将副本id下发给复制微服务，这是与普通复制之前的区别
    :param user_id: 用户id
    :param replication_req: 反向复制请求体
    """
    check_target_cluster(replication_req)
    copy_id = replication_req.copy_id
    log.info(f"Execute reverse replication, copy id: {copy_id}")
    copy_info = query_copy_by_id(copy_id)
    domain_id_list = get_domain_id_list(copy_id)
    if not copy_info:
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, message="copy is null.")
    copy_format = json.loads(copy_info.properties).get("format", "0")
    log.info(f"copy format:{copy_format}")
    policy = build_policy(replication_req, copy_info.backup_type)
    if not ReplicationClient.is_hcs_service():
        log.info(f'Save temp user info to database')
        save_temp_user_info(policy, replication_req)
    cur_esn = copy_info.device_esn
    resource_id = copy_info.resource_id
    if not ReplicationClient.is_hcs_service():
        ReplicationClient.check_before_manual_replicate(replication_req, resource_id)
    resource_obj = copy_info.resource_properties
    job_params = {
        'copy_format': copy_format,
        'copy_info': copy_info,
        'resource_obj': resource_obj,
        'domain_id_list': domain_id_list,
        'policy': policy,
        'resource_id': resource_id,
        'cur_esn': cur_esn,
        'user_id': user_id
    }
    switch_rep_by_copy_type(copy_id, copy_info, job_params)


def switch_rep_by_copy_type(copy_id, copy_info, job_params):
    if copy_info.backup_type in {BackupTypeEnum.log.value}:
        copy_list = get_copy_list(copy_info)
        log_copy_list = []
        data_copy_list = []
        for item in copy_list:
            backup_type = item.get('backup_type')
            if backup_type == BackupTypeEnum.log.value:
                log_copy_list.append(item)
            else:
                data_copy_list.append(item)
        copy_dict = {}
        for item in data_copy_list:
            format_value = json.loads(item["properties"]).get("format", "0")
            copy_dict.setdefault(format_value, []).append(item.get("uuid", ""))
        for copy_format, copy_list in copy_dict.items():
            copy_list.reverse()  # Reverse copy list
            update_policy(job_params, PolicyActionEnum.replication.value)
            fill_job_param(job_params, copy_format, copy_list, False)
        if log_copy_list:
            copy_list = [item.get("uuid", "") for item in log_copy_list]
            copy_list.reverse()  # Reverse copy list
            copy_format = json.loads(log_copy_list[0]["properties"]).get("format", "0")
            update_policy(job_params, PolicyActionEnum.replication_log.value)
            fill_job_param(job_params, copy_format, copy_list, True)
    else:
        request_id = str(uuid.uuid4())
        job_params.update({
            'request_id': request_id,
            'copy_list': [],
            'is_delete_temp_user': True,
            'copy_id': copy_id
        })
        create_reverse_copy_job(job_params)


def update_policy(job_params, action):
    policy = job_params['policy']
    policy.update({
        'action': action
    })
    job_params.update({
        'policy': policy
    })


def fill_job_param(job_params, copy_format, copy_list, is_delete_temp_user):
    request_id = str(uuid.uuid4())
    log.info(f"Start create reverse copy task, copy_format:{copy_format}, copy_list:{copy_list},"
             f" request_id:{request_id}")
    job_params.update({
        'copy_format': copy_format,
        'request_id': request_id,
        'copy_list': copy_list,
        'is_delete_temp_user': is_delete_temp_user
    })
    create_reverse_copy_job(job_params)


def create_reverse_copy_job(job_params):
    request_id = job_params['request_id']
    copy_format = job_params['copy_format']
    copy_list = job_params['copy_list']
    is_delete_temp_user = job_params['is_delete_temp_user']
    copy_info = job_params['copy_info']
    resource_obj = job_params['resource_obj']
    domain_id_list = job_params['domain_id_list']
    policy = job_params['policy']
    resource_id = job_params['resource_id']
    cur_esn = job_params['cur_esn']
    user_id = job_params['user_id']
    copy_id = job_params.get('copy_id', None)

    message_payload = {
        "request_id": request_id,
        "copy_id": copy_list[0] if copy_list else copy_id,
        "is_delete_temp_user": is_delete_temp_user,
        "resource_id": resource_id,
        "execute_type": ExecuteType.MANUAL.value,
        "policy": policy,
        "unit": copy_info.storage_unit_id,
        "resource_obj": json.loads(resource_obj),
        "same_chain_copies": copy_list if copy_list else get_directory_copy_same_chain_copies(copy_info),
        "copy_type": copy_format,
        "rep_mode": ReplicationConstants.REP_MODE_MANUAL
    }

    JobClient.create_job(
        device_esn=cur_esn,
        request_id=request_id,
        user_id=user_id,
        resource_obj=json.loads(resource_obj),
        job_type=JobType.COPY_REPLICATION.value,
        domain_id_list=domain_id_list,
        message=JobMessage(topic="initialize.replication", payload=message_payload),
        job_extend_params={"multiClusterQueue": True},
        enable_stop=True,
        target_name=None,
        target_location=None
    )


def get_copy_list(copy_info):
    same_chain_copies = get_directory_copy_same_chain_copies(copy_info)
    same_chain_copies.reverse()
    result = []
    for item_id in same_chain_copies:
        item = query_copy_by_id(item_id)
        if item:
            item = item.__dict__
            result.append(item)
    return result


def check_target_cluster(replication_req):
    target_cluster_info = query_target_cluster_by_id(replication_req.external_system_id)
    if not target_cluster_info:
        raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM,
                                   message="The replication target cluster does not exist.")
    log.info(f'query target cluster, role:{target_cluster_info.role}, uuid:{replication_req.external_system_id}')
    if target_cluster_info.role != 1:
        raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM,
                                   message="The target cluster role is not a replication cluster.")


def save_temp_user_info(policy, replication_req):
    user_info = {
        "uuid": 'temp_' + str(uuid.uuid4()),
        "user_id": replication_req.user_id,
        "username": replication_req.username,
        "password": encrypt(replication_req.password),
        "sla_id": "",
        "policy_id": policy.get("uuid"),
        "user_type": replication_req.userType
    }
    with database.session() as session:
        session.add(RepSlaUser(**user_info))
    clear(replication_req.password)
    clear(user_info.get("password"))


def get_copy_esn(replication_req: ReplicationRequest):
    copy_id = replication_req.copy_id
    log.info("copy_id" + copy_id)
    copy_info = query_copy_by_id(copy_id)
    if not copy_info:
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, message="copy is null.")
    return copy_info.device_esn


def get_directory_copy_same_chain_copies(copy_info):
    copy_list = []
    properties = json.loads(copy_info.properties)
    copy_format = properties.get("format")
    if copy_format != CopyFormatEnum.INNER_DIRECTORY.value:
        log.info(f"Copy: {copy_info.uuid} format {copy_format} is not directory.")
        return copy_list
    copy_list = get_same_chain_copies(copy_info)
    log.info(f"Directory copy: {copy_info.uuid} same chain copies: {copy_list}")
    return copy_list


def build_policy(replication_req: ReplicationRequest, backup_type: int):
    if backup_type == 4:
        action = PolicyActionEnum.replication_log.value
    else:
        action = PolicyActionEnum.replication.value
    policy = {
        "uuid": 'temp_' + str(uuid.uuid4()),
        "name": "reverse_replication",
        "action": action,
        "type": PolicyTypeEnum.replication.value,
        "retention": {
            "retention_type": replication_req.retention_type,
            "retention_duration": replication_req.retention_duration,
            "duration_unit": replication_req.duration_unit
        },
        "schedule": {
            "start_time": datetime.now(tz=timezone.utc).strftime("%Y-%m-%dT%H:%M:%S")
        }
    }
    if ReplicationClient.is_hcs_service():
        policy.update(get_hcs_ext_parameters(replication_req))
    else:
        policy.update({
            "ext_parameters": {
                "external_system_id": replication_req.external_system_id,
                "link_deduplication": replication_req.link_deduplication,
                "link_compression": replication_req.link_compression,
                "alarm_after_failure": replication_req.alarm_after_failure,
                "user_info": {
                    "user_id": replication_req.user_id,
                    "username": replication_req.username,
                    "password": replication_req.password
                },
                "storage_info": {
                    "storage_type": replication_req.storage_type,
                    "storage_id": replication_req.storage_id
                }
            }
        })
    return policy


def get_hcs_ext_parameters(replication_req: ReplicationRequest):
    return {
        "ext_parameters": {
            "external_system_id": replication_req.external_system_id,
            "link_deduplication": replication_req.link_deduplication,
            "link_compression": replication_req.link_compression,
            "alarm_after_failure": replication_req.alarm_after_failure,
            "project_id": replication_req.user_id,
            "replication_target_mode": 3,
            "storage_info": {
                "storage_type": replication_req.storage_type,
                "storage_id": replication_req.storage_id
            }
        }
    }
