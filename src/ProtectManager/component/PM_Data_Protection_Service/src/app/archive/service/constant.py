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

from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.exception.common_error_codes import BaseErrorCode
from app.copy_catalog.common.copy_status import CopyStatus


class ArchiveConstant(object):
    TOPIC_ARCHIVE_SUCCESS = "protection.archive.success"
    TASK_ARCHIVE_DONE_TOPIC = "protection.archive.done"
    ARCHIVE_LOCK_RESPONSE = "Archive_LockResponse"
    APPLICATION_SUPPORT_TAPE_AUTO_INDEX = []
    NOT_ALLOW_ARCHIVE_COPY_STATUS = [CopyStatus.INVALID.value, CopyStatus.DELETING.value, CopyStatus.DELETEFAILED.value]
    LOG_ARCHIVE_SUB_TYPE = [ResourceSubTypeEnum.TDSQL, ResourceSubTypeEnum.TDSQL_CLUSTER,
                            ResourceSubTypeEnum.TDSQL_CLUSTER_INSTANCE, ResourceSubTypeEnum.TDSQL_CLUSTER_GROUP]


class ArchiveErrorCode(str, Enum):
    CANCELLED_ERROR_CODE = "1677934343"


class ArchiveErrorCodeDict(BaseErrorCode):
    EXIST_ARCHIVE_JOB = {
        "code": "1677935132",
        "message": "Archive job exist."
    }
    EXIST_ARCHIVE_COPY = {
        "code": "1677935131",
        "message": "Archive copy exist."
    }
    NOT_SUPPORT_TAPE_ARCHIVE = {
        "code": "1677932053",
        "message": "Not support tape archive."
    }
