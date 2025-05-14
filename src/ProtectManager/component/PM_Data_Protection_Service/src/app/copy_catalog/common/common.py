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
from enum import Enum


class HttpStatusDetail:
    detail_404 = "Not Found"
    detail_500 = "Internal Error"


class ResourceStatus(Enum):
    EXIST = "EXIST"
    NOT_EXIST = "NOT_EXIST"


class IndexStatus(Enum):
    UNINDEXED = "Unindexed"
    INDEXED = "Indexed"
    INDEXING = "Indexing"
    INDEX_DELETING = "Index_deleting"
    INDEX_FAIL = "Index_fail"
    UNSUPPORT = "Unsupport"


class GenIndexType(str, Enum):
    # 手动创建副本索引
    MANUAL = "manual"
    # 自动创建副本索引
    AUTO = "auto"


class CommonOperationID(Enum):
    # 创建副本索引失败告警事件
    EVENT_CREATE_COPIES_INEDEX_FAILED = "0x64033A0001"

    # 归档副本创建索引失败告警
    ARCHIVE_COPIES_INDEX_FAILED = "0x5F025D0001"


class OperationLabel(Enum):
    INDEX_RESPONSE_ERROR_LABEL = "index_response_error_label"


class BrowseMountStatus(Enum):
    UMOUNT = "Umount"
    MOUNTED = "Mounted"
    MOUNTING = "Mounting"
    MOUNT_FAIL = "Mount_fail"
    MOUNT_DELETING = "Mount_deleting"
    MOUNT_DELETE_FAIL = "Mount_delete_fail"
    UNSUPPORTED = "Unsupported"


class CopyRetentionTypeLabel(str, Enum):
    PERMANENT = "copy_retention_type_permanent_label"
    TEMPORARY = "copy_retention_type_temporary_label"


class CopyRetentionDurationUnitLabel(str, Enum):
    DAYS = "copy_retention_duration_unit_days_label"
    WEEKS = "copy_retention_duration_unit_weeks_label"
    MONTHS = "copy_retention_duration_unit_months_label"
    YEARS = "copy_retention_duration_unit_year_label"


class CopyWormValidityTypeLabel(str, Enum):
    COPY_RETENTION_TIME_CONSISTENT = "common_worm_same_validity_label"
    CUSTOM_RETENTION_TIME = "common_worm_custom_validity_label"


class CopyFeatureEnum(int, Enum):
    INDEX = 0
    RESTORE = 1
    INSTANT_RESTORE = 2
    MOUNT = 3
    WORM = 4


class CopyConstants:
    # 1个月（秒）
    MONTH_IN_SECOND = 30 * 24 * 60 * 60
    # 2个小时（秒）
    TWO_HOUR_SECOND = 2 * 60 * 60
    # 过期任务最大间隔时间7天（秒）
    EXPIRE_JOB_MAX_INTERVAL_SECOND = 7 * 24 * 60 * 60
    COPY_EXPIRE_FAILED = "0x64033A0002"
    SNAPSHOT_EXPIRE_FAILED = "0x64033A0003"
