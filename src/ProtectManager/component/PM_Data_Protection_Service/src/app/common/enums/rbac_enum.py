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


class ResourceSetTypeEnum(str, Enum):
    RESOURCE = "RESOURCE"
    COPY = "COPY"
    QOS = "QOS"
    SLA = "SLA"
    RESOURCE_GROUP = "RESOURCE_GROUP"
    AGENT = "AGENT"
    JOB = "JOB"
    ALARM = "ALARM"
    EVENT = "EVENT"
    REPORT = "REPORT"
    EXERCISE = "EXERCISE"
    JOB_LOG = "JOB_LOG"
    FILE_SET_TEMPLATE = "FILE_SET_TEMPLATE"
    AIR_GAP = "AIR_GAP"
    PREVENT_EXTORTION_AND_WORM = "PREVENT_EXTORTION_AND_WORM"
    DESENSITIZATION = "DESENSITIZATION"
    LIVE_MOUNT_POLICY = "LIVE_MOUNT_POLICY"


class ResourceSetScopeModuleEnum(str, Enum):
    VMWARE = "VMware"
    AGENT = "AGENT"
    COPY = "COPY"
    QOS = "QOS"


class OperationTypeEnum(str, Enum):
    CREATE = "create"
    DELETE = "delete"
    MODIFY = "modify"
    QUERY = "query"


class AuthOperationEnum(str, Enum):
    MANAGE_CLIENT = "manageClient"
    MANAGE_RESOURCE = "manageResource"
    BACKUP = "backup"
    REPLICATION = "replication"
    ARCHIVE = "archive"
    SLA = "sla"
    SPEED_LIMIT_STRATEGY = "speedLimitStrategy"
    ORIGINAL_RESTORE = "originalRestore"
    NEW_RESTORE = "newRestore"
    IMMEDIATELY_RESTORE = "immediatelyRestore"
    RESTORE_EXERCISE = "restoreExercise"
    AIR_GAP = "airGap"
    LIVE_MOUNT = "liveMount"
    PREVENT_EXTORTION_AND_WORM = "preventExtortionAndWorm"
    DESENSITIZATION = "desensitization"
    copyDelete = "copyDelete"
    copyIndex = "copyIndex"
    liveMountPolicy = "liveMountPolicy"
    REPORT = "report"
