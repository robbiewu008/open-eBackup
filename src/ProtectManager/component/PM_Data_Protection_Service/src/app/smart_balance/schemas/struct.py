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
from pydantic import Field
from pydantic.main import BaseModel


class ExecuteCluster(BaseModel):
    esn: str = Field(alias='esn')
    total_capacity: str = Field(alias='totalCapacity')
    used_capacity: str = Field(alias='usedCapacity')
    backup_type: str = Field(alias='backupType')
    resource_id: str = Field(alias='resourceId')
    # 历史运行的备份任务数量
    history_backup_job_count: str = Field(alias='historyBackupJobCount')
    # 成功运行的备份任务数量
    history_success_backup_job_count: str = Field(alias="historySuccessBackupJobCount")
    # 固定值，规格，允许同时运行任务数
    running_task_spec_count: str = Field(alias='runningTaskSpecCount')
    # 运行中的备份任务数量
    running_backup_job_count: str = Field(alias="runningBackupJobCount")
    # 运行中的总的备份任务数量
    running_total_job_count: str = Field(alias="runningTotalJobCount")
    # 需要实际运行的flag
    running_flag: str = Field(alias="flag")
    # 存储池粒度的信息
    # id：pool id， totalcapacityPool, usedCapacitPool, historyBackupJobCountPool,
    # historySuccessBackupJobCountPool, runningTaskSpecCountPool, runningTotalJobCountPool
    units: str = Field(alias="unit")
