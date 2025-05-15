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
from app.common.enums.sla_enum import PolicyActionEnum


class BackupWorkflowConstants(object):
    # 本次备份任务的任务id
    JOB_ID = "job_id"
    # 本次备份任务的任务类型
    JOB_TYPE = "job_type"
    # 备份的chain_id
    CHAIN_ID = "chain_id"
    # 资源id
    RESOURCE_ID = "resource_id"
    # 资源组id
    RESOURCE_GROUP_ID = "resource_group_id"
    # 资源名称
    RESOURCE_NAME = "resource_name"
    # 资源对象
    RESOURCE = "resource"
    # 备份类型
    BACKUP_TYPE = "backup_type"
    # 本次备份的备份策略
    POLICY = "policy"
    # 保护对象绑定的sla id
    SLA_ID = "sla_id"
    # 保护对象绑定的SLA
    SLA = "sla"
    # 本次执行备份的保护对象
    PROTECTED_OBJECT = "protected_object"
    # 备份执行类型，自动执行还是手动执行
    EXECUTE_TYPE = "execute_type"
    # 时间窗开始时间
    TIME_WINDOW_START = "time_window_start"
    # 时间窗结束时间
    TIME_WINDOW_END = "time_window_end"
    # 本次备份任务的超时时间
    TIMEOUT_TIME = "timeout_time"
    # 是否是首次备份
    FIRST_BACKUP = "first_backup"
    # 手动备份user_id
    CURRENT_OPERATE_USER_ID = "current_operate_user_id"
    # 是否开启重试
    AUTO_RETRY = "auto_retry"
    # 重试次数
    RETRY_TIMES = "retry_times"
    # 重试间隔
    WAIT_MINUTES = "wait_minutes"
    # 任务取消回调URL
    JOB_CANCEL_CALLBACK_URL = 'callback.cancel'
    # 任务取消回调数据
    JOB_CANCEL_CALLBACK_EXECUTE_TYPE = 'callback.data.backup.execute.type'
    # 副本名称
    COPY_NAME = "copy_name"
    # 副本id
    COPY_ID = "copy_id"
    # 增量备份类型
    INCREMENT_BACKUP_TYPES = (PolicyActionEnum.permanent_increment,
                              PolicyActionEnum.difference_increment,
                              PolicyActionEnum.cumulative_increment)
    # 所有备份类型
    ALL_BACKUP_TYPES = (PolicyActionEnum.full,
                        PolicyActionEnum.log,
                        PolicyActionEnum.permanent_increment,
                        PolicyActionEnum.difference_increment,
                        PolicyActionEnum.cumulative_increment)
    # 下一次备份类型
    KEY_NEXT_BACKUP_TYPE = 'next_backup_type'
    # 变更类型原因
    KEY_NEXT_BACKUP_CAUSE = 'next_backup_change_cause'
    # 本次备份策略是变更原因
    CAUSE_OF_BACKUP_TYPE_CHANGE = 'cause_of_change'
    # 备份策略变更原因1： 保护对象中ext_parameters->by_next_backup参数
    BY_NEXT_BACKUP = 'by_next_backup'
    # 备份策略变更原因2： 保护对象中完整性检查不过
    BY_IN_CONSISTENT = 'by_in_consistent'
