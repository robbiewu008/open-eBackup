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
from datetime import datetime
from typing import List

from app.ai_sorter.model import VM
from app.base.db_base import database
from app.common.exter_attack import exter_attack
from app.common.enums.protected_object_enum import Status
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.schemas.job_schemas import JobSchema
from app.protection.object.models.projected_object import ProtectedObject


def get_vmware_objects() -> List[ProtectedObject]:
    """
    查找PM数据库，获取所有类型为vmware虚拟机的保护对象
    :return: 保护对象列表
    """
    with database.session() as session:
        condition = {
            ProtectedObject.sub_type == ResourceSubTypeEnum.VirtualMachine.value,
            ProtectedObject.status == Status.Active.value
        }
        vmware_protect_objects = session.query(ProtectedObject) \
            .filter(*condition).all()
        return vmware_protect_objects


@exter_attack
def get_vmware_object_from_job(job: JobSchema) -> ProtectedObject:
    """
    查找PM数据库，找到job对应的保护对象

    :param job: job信息
    :return: job对应的保护对象
    """
    with database.session() as session:
        vmware_protect_object = session.query(ProtectedObject).filter(
            ProtectedObject.resource_id == job.source_id
        ).first()
        return vmware_protect_object


@exter_attack
def parse_vm_from_job(job: JobSchema) -> VM:
    message = json.loads(job.message)
    internal = message["payload"]["policy"]["schedule"]["interval"]
    interval_unit = message["payload"]["policy"]["schedule"][
        "interval_unit"]
    backup_freq = 0
    if interval_unit == 'm':
        backup_freq = internal * 60
    elif interval_unit == 'h':
        backup_freq = internal * 60 * 60
    elif interval_unit == 'd':
        backup_freq = internal * 24 * 60 * 60

    protect_obj = get_vmware_object_from_job(job)

    if protect_obj.earliest_time is None:
        earliest_time = datetime.utcnow().timestamp()
    else:
        earliest_time = datetime.strptime(
            str(protect_obj.earliest_time), '%Y-%m-%d %H:%M:%S.%f').timestamp()
    vm_item = VM(job.job_id,
                 job.source_name,
                 backup_freq,
                 message["traffic"]["priority"],
                 earliest_time)

    return vm_item
