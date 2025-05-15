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
import time

from app.common.clients.resource_client import ResourceClient
from app.common.deploy_type import DeployType
from app.common.enums.resource_enum import ResourceSubTypeWithOrderEnum
from app.common.log.kernel import convert_storage_type
from app.copy_catalog.common.common import CopyConstants
from app.common.clients.alarm.alarm_client import AlarmClient
from app.common.clients.alarm.alarm_schemas import SendAlarmReq
from app.common.enums.alarm_enum import AlarmSourceType


def send_copy_expire_failed_alarm(resource_sub_type: str, resource_id: str, resource_name: str, copy_id: str,
                                  job_id: str):
    timestamp = int(time.time())
    if DeployType().is_cyber_engine_deploy_type():
        resource = ResourceClient.query_resource(resource_id=resource_id)
        env_info = {} if resource == {} else ResourceClient.query_resource(resource_id=resource.get("root_uuid"))
        alarm_id = CopyConstants.SNAPSHOT_EXPIRE_FAILED
        params = ",".join([env_info.get("name", ""), env_info.get("uuid", ""),
                           convert_storage_type(env_info.get("sub_type", "")),
                           resource.get("parent_name", ""), resource.get("parent_uuid", ""),
                           resource.get("uuid", ""), resource_name, copy_id, job_id])
    else:
        alarm_id = CopyConstants.COPY_EXPIRE_FAILED
        params = str(ResourceSubTypeWithOrderEnum.get_order(
            resource_sub_type)) + "," + resource_name + "," + copy_id + "," + job_id
    AlarmClient.send_alarm(SendAlarmReq(
        alarmId=alarm_id,
        params=params,
        alarmSource="localhost",
        createTime=timestamp,
        sequence=timestamp,
        sourceType=AlarmSourceType.COPY_CATALOG,
        resourceId=resource_id
    ))
