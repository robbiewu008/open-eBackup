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


class AlarmLevel(str, Enum):
    INVALID = "INVALID"
    INFO = "INFO"
    WARNING = "WARNING"
    MINOR = "MINOR"
    MAJOR = "MAJOR"
    CRITICAL = "CRITICAL"


class AlarmSourceType(str, Enum):
    USER = "user"
    ALARM = "alarm"
    EVENT = "event"
    NOTIFY = "notify"
    RESOURCE = "resource"
    PROTECTION = "protection"
    RECOVERY = "recovery"
    BACKUP_CLUSTER = "backupCluster"
    NETWORK_ENTITY = "networkEntity"
    CERTIFICATE = "certificate"
    CLUSTER = "cluster"
    LICENSE = "license"
    REPOSITORY = "repository"
    KMS = "kms"
    SLA = "sla"
    PROTECTED_OBJECT = "protectedObject"
    SCHEDULER = "scheduler"
    LIVE_MOUNT = "liveMount"
    RESTORE = "restore"
    COPY_CATALOG = "copyCatalog"
    REPLICATION = "replication"
    LOCAL_STORAGE = "localStorage"
