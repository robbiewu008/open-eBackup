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
from app.common.exception.common_error_codes import BaseErrorCode


class ProtectionErrorCodes(BaseErrorCode):
    SLA_NAME_DUPLICATE = {
        "code": "1677931521",
        "message": "SLA name repeated"
    }

    SLA_COUNT_OVER_LIMIT = {
        "code": "1677931522",
        "message": "SLA count reached or exceeded the limit {0}"
    }

    ARCHIVE_POLICY_COUNT_OVER_LIMIT = {
        "code": "1677931523",
        "message": "The number of archiving policies in the SLA cannot exceed {0}"
    }

    REPLICATION_POLICY_COUNT_OVER_LIMIT = {
        "code": "1677931524",
        "message": "The number of replication policies in the SLA cannot exceed {0}"
    }

    SLA_INVALID_OPERATION = {
        "code": "1677931525",
        "message": "The operation is not allowed."
    }

    SLA_INVALID_MODIFY = {
        "code": "1677931526",
        "message": "SLA params cannot be modified"
    }

    SLA_BOUND_RESOURCE = {
        "code": "1677931527",
        "message": "SLA already bound resource, can not delete"
    }

    START_TIME_NOT_IN_TIME_WINDOW = {
        "code": "1677931528",
        "message": "Backup start time not in time windows"
    }

    START_TIME_CAN_NOT_EARLIER_NOW = {
        "code": "1677931529",
        "message": "The first start time is earlier than the current time"
    }

    OPERATION_INTERVAL_CAN_NOT_MORE_THAN_RETENTION = {
        "code": "1677931530",
        "message": "Operation interval can not more than retention"
    }

    QOS_OPERATION_FAIL = {
        "code": "1677932034",
        "message": "qos is used."
    }

    SLA_APPLICATION_NOT_SUPPORT_POLICY_ACTION = {
        "code": "1677748737",
        "message": "SLA application not support this policy action."
    }

    START_TIME_INVALID_OPERATION = {
        "code": "1677931531",
        "message": "Backup start time is invalid."
    }
    QOS_MAX_COUNT = {
        "code": "1677932033",
        "message": "qos info count is max."
    }
    QOS_NAME_REPEAT = {
        "code": "1677932037",
        "message": "qos name is repeat."
    }
    K8S_RULE_COUNT_OVER_LIMIT = {
        "code": "1677932039",
        "message": "Kubernetes rule count reaches or exceeds the limit {0}"
    }
    CLOUDBACKUP_SYNCHRONOUS_REPLICATION_SECONDARY = {
        "code": "1677747458",
        "message": "cloudbackup synchronous replication secondary can't protect "
    }
    CYBERENGINE_HYPER_METRO_SECONDARY_CANNOT_BACKUP = {
        "code": "1677747457",
        "message": "cyberengine hyper metro secondary can't manual backup"
    }
    SINGLE_RESOURCE_COPIES_COUNT_OVER_LIMIT = {
        "code": "1677932042",
        "message": "Single resource copies count is over 4096."
    }
    CYBERENGINE_SYNCHRONOUS_REPLICATION_SECONDARY = {
        "code": "1677747461",
        "message": "oceancyber synchronous replication secondary can't manual backup."
    }
    CLOUDBACKUP_SMART_MOBILITY_CANNOT_BACKUP = {
        "code": "1677747464",
        "message": "cloudbackup smart mobility can't manual backup."
    }
    LOG_BACKUP_IS_THE_FIRST_BACKUP = {
        "code": "1677747463",
        "message": "there is no copy supports manual log backup."
    }
