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


class JobStatus(Enum):
    """
    0-初始化
    1-运行中
    2-完成
    3-已终止
    4-失败
    99-其它
    """
    init = 0
    running = 1
    completed = 2
    terminated = 3
    failure = 4
    other = 99
    READY = "READY"
    PENDING = "PENDING"
    RUNNING = "RUNNING"
    SUCCESS = "SUCCESS"
    PARTIAL_SUCCESS = "PARTIAL_SUCCESS"
    FAIL = "FAIL"
    ABORTED = "ABORTED"
    ABORTING = "ABORTING"
    ABORT_FAILED = "ABORT_FAILED"
    ABNORMAL = "ABNORMAL"
    CANCELLED = "CANCELLED"
    DISPATCHING = "DISPATCHING"
    REDISPATCH = "REDISPATCH"

    def success(self):
        return self in [JobStatus.SUCCESS, JobStatus.PARTIAL_SUCCESS]

    def need_retry(self):
        return self in [JobStatus.FAIL]


class JobLogLevel(Enum):
    INFO = "info"
    WARNING = "warning"
    ERROR = "error"
    FATAL = "fatal"


class JobType(Enum):
    BACKUP = "BACKUP"
    GROUP_BACKUP = "GROUP_BACKUP"
    RESTORE = "RESTORE"
    INSTANT_RESTORE = "INSTANT_RESTORE"
    LIVE_MOUNT = "live_mount"
    COPY_REPLICATION = "copy_replication"
    ARCHIVE = "archive"
    CLOUD_ARCHIVE_RESTORE = "cloud_archive_restore"
    COPY_DELETE = "COPY_DELETE"
    COPY_EXPIRE = "COPY_EXPIRE"
    ARCHIVE_IMPORT = "archive_import"
    MIGRATE = "migrate"
    RESOURCE_SCAN = "resource_scan"
    RESOURCE_PROTECTION = "resource_protection"
    RESOURCE_PROTECTION_MODIFY = "resource_protection_modify"
    MANUAL_SCAN_RESOURCE = "job_type_manual_scan_resource"
