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
from typing import List

from app.ai_sorter.model import VM
from app.common.schemas.job_schemas import JobSchema
from app.protection.object.models.projected_object import ProtectedObject
from app.common.toolkit import query_job_list
from app.common import logger

log = logger.get_logger(__name__)


def get_backup_jobs_per_vm(vm_obj: ProtectedObject, min_jobs_per_vm: int) -> \
        List[JobSchema]:
    job_list = []
    job_condition_map = {
        'sourceName': vm_obj.name,
        'statusList': ['SUCCESS'],
        'types': ['BACKUP'],
        'orderBy': 'startTime',
        'orderType': 'asc',
        'pageSize': 200
    }
    response = query_job_list(job_condition_map)
    if response is None:
        return job_list

    paged_job_bos = json.loads(response)
    total_count = paged_job_bos['totalCount']
    if total_count < min_jobs_per_vm:
        return job_list

    for job in paged_job_bos['records']:
        job_schema = JobSchema(**job)
        job_list.append(job_schema)
    return job_list


def get_job_speed(job: JobSchema):
    """
    返回指定任务的吞吐量

    :param job: 任务
    :return: 吞吐量，MB/s; -1则为失败。
    """
    job_size = parse_job_size(job)
    if job_size == -1:
        return -1
    job_time_range = (job.end_time - job.start_time) / 1000
    try:
        return job_size / max(job_time_range, 0.1)
    except ZeroDivisionError:
        log.info(f"job_time_range is near zero, "
                 f"end_time: {job.end_time}, start_time:{job.start_time}")
        return job_size / 0.1


def parse_job_size(job_inf: JobSchema):
    """
    解析job size数据，并全部统一为MB为单位的大小
    :param job_inf: job信息数据
    :return: 通过远程调用查询成功备份的历史任务
    """
    prev_job_size_extend_str = json.loads(job_inf.extend_str)
    try:
        prev_job_size_inf = prev_job_size_extend_str['dataAfterReduction']
    except KeyError:
        log.info(f"read job size error! no such key:[dataAfterReduction] in "
                 f"extend string!")
        return -1

    # 将job size统一转化为MB
    if 'TB' in prev_job_size_inf:
        prev_job_size = float(prev_job_size_inf.rstrip('TB').rstrip()) * 1024 * 1024
    elif 'GB' in prev_job_size_inf:
        prev_job_size = float(prev_job_size_inf.rstrip('GB').rstrip()) * 1024
    elif 'MB' in prev_job_size_inf:
        prev_job_size = float(prev_job_size_inf.rstrip('MB').rstrip())
    elif 'KB' in prev_job_size_inf:
        prev_job_size = float(prev_job_size_inf.rstrip('KB').rstrip()) / 1024
    elif 'B' in prev_job_size_inf:
        prev_job_size = float(prev_job_size_inf.rstrip('B').rstrip()) / 1024 / 1024
    else:
        log.info(
            f'error, unsupported unit in [{prev_job_size_inf}]! for job size')
        prev_job_size = -1

    return prev_job_size


def get_prev_jobs(vm_item: VM, window_len: int) -> List[JobSchema]:
    """
    获取成功备份的历史任务
    :param window_len: 备份历史成功任务数目
    :param vm_item: VM类，含job调度信息
    :return: 通过远程调用查询成功备份的历史任务
    """
    response = query_job_list(
        {'fromStartTime': int(
            datetime.datetime.utcnow().timestamp()
            - float(vm_item.schedule_freq * window_len)) * 1000,
         'sourceName': vm_item.name,
         'statusList': ['SUCCESS'], 'types': ['BACKUP'],
         'orderBy': 'startTime', 'orderType': 'asc'})

    paged_job_bos = json.loads(response)
    job_list = []
    for job in paged_job_bos['records']:
        job_schema = JobSchema(**job)
        job_list.append(job_schema)

    return job_list


def get_prev_job_speed_list(vm_item: VM, window_len: int) -> List[float]:
    pre_job_list = get_prev_jobs(vm_item, window_len)

    prev_job_speed_list = []
    for prev_job in pre_job_list:
        job_speed = get_job_speed(prev_job)
        if job_speed < 0:
            continue
        prev_job_speed_list.append(job_speed)
    return prev_job_speed_list
