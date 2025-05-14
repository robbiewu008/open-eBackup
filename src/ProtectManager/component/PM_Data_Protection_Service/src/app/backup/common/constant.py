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
from app.common.enums.resource_enum import ResourceSubTypeEnum


class ProtectionConstant(object):
    SECONDS_OF_ONE_DAY = 24 * 3600
    TOPIC_BACKUP_TIMEOUT_CHECK = "protection.backup.timeout.check"
    TOPIC_BACKUP_LOCKED = "protection.backup.locked"
    TOPIC_BACKUP_SUCCESS = "protection.backup.success"
    TOPIC_BACKUP_UNLOCKED = "protection.backup.unlocked"
    DATE_TIME_FORMATTER = "%Y-%m-%d %H:%M:%S"
    TIME_FORMATTER = "%H:%M:%S"
    EVENT_BACKUP_EXECUTION_EXCEEDS_TIME_WINDOW = "0x106403350001"
    ALARM_BACKUP_EXECUTION_EXCEEDS_TIME_WINDOW = "0x106403330002"
    ALARM_SNAPSHOT_EXECUTION_EXCEEDS_TIME_WINDOW = "0x106403350002"
    TOPIC_REPLICA_SUCCESS = "copy.replica.success"
    CONSISTENT_KEY = "consistent_status"
    CONSISTENT_RESULTS = "consistent_results"
    CONSISTENT = "consistent"
    INCONSISTENT = "inconsistent"
    CLUSTER_CONFIG = "cluster-conf"
    CLUSTER_ESN = "CLUSTER_ESN"


class CommonOperationID(object):
    # 创建SLA
    CREATED_SLA = "0x206403330001"

    # 修改SLA
    MODIFY_SLA = "0x206403330003"

    # 移除SLA
    REMOVE_SLA = "0x206403330002"

    # 创建K8S Rule
    CREATE_K8S_RULE = "0x206403350007"

    # 修改K8S Rule
    UPDATE_K8S_RULE = "0x206403350009"

    # 删除K8S Rule
    DELETE_K8S_RULE = "0x206403350008"


class ProtectionBackupJobSteps(object):
    # 备份检查失败
    BACKUP_CHECK_FAILED = "job_log_protection_backup_execute_check_failed_label"


class CloudBackupConstant:
    MAX_SYNTHETIC_FULL_COPY_PERIOD = 100
    MIN_SYNTHETIC_FULL_COPY_PERIOD = 1


class AntiRansomwareAlarm:
    SMART_MOBILITY_CAN_NOT_BACKUP_ALARM_ID = "0x6403350003"
