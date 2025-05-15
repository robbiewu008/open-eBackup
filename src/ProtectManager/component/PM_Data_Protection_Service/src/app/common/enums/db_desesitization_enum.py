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

from app.common.enums.job_enum import JobStatus


class TaskTypeEnum(str, Enum):
    Identification = "identification"
    Desesitization = "desesitization"
    ConfirmPolicy = "confirmpolicy"


class TaskStatusEnum(str, Enum):
    Start = "Start"
    Execute = "Execute"
    SuccessEnd = "SuccessEnd"
    FailEnd = "FailEnd"
    Aborted = "Aborted"
    Aborting = "Aborting"


class IdentificationStatusEnum(str, Enum):
    NotIdentified = "NOT_IDENTIFIED"
    Identifing = "IDENTIFING"
    Identified = "IDENTIFIED"
    FailIdentified = "FAILED_IDENTIFIED"
    Aborted = "ABORTED"
    Aborting = "ABORTING"


class DesesitizationStatusEnum(str, Enum):
    NotDesesitized = "NOT_DESESITIZE"
    Desesitizing = "DESESITIZING"
    Desesitized = "DESESITIZED"
    FailDesesitized = "FAILED_DESESITIZED"
    Aborted = "ABORTED"
    Aborting = "ABORTING"


IdentificationStatusMap = {
    "Start": IdentificationStatusEnum.NotIdentified,
    "Execute": IdentificationStatusEnum.Identifing,
    "SuccessEnd": IdentificationStatusEnum.Identified,
    "FailEnd": IdentificationStatusEnum.FailIdentified,
    "Aborting": IdentificationStatusEnum.Aborting,
    "Aborted": IdentificationStatusEnum.Aborted
}

JobStatusMap = {
    "Start": JobStatus.RUNNING.value,
    "Execute": JobStatus.RUNNING.value,
    "SuccessEnd": JobStatus.SUCCESS.value,
    "FailEnd": JobStatus.FAIL.value,
    "Aborting": JobStatus.ABORTING.value,
    "Aborted": JobStatus.ABORTED.value
}

DesesitizationStatusMap = {
    "Start": DesesitizationStatusEnum.NotDesesitized,
    "Execute": DesesitizationStatusEnum.Desesitizing,
    "SuccessEnd": DesesitizationStatusEnum.Desesitized,
    "FailEnd": DesesitizationStatusEnum.FailDesesitized,
    "Aborting": DesesitizationStatusEnum.Aborting,
    "Aborted": DesesitizationStatusEnum.Aborted
}
