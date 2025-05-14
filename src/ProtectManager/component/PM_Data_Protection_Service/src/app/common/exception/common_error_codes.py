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


class BaseErrorCode(dict, Enum):
    pass


class CommonErrorCodes(BaseErrorCode):
    OBJ_NOT_EXIST = {
        "code": "1677929217",
        "message": "object does not exist."
    }
    ERR_PARAM = {
        "code": "1677929218",
        "message": "The parameter is incorrect"
    }
    OPERATION_FAILED = {
        "code": "1677929219",
        "message": "The object already exists."
    }
    ILLEGAL_PARAMS = {
        "code": "1677929220",
        "message": "Illegal Params."
    }
    REP_COPY_CANNOT_ARCHIVE = {
        "code": "1677932048",
        "message": "replicated copy can not archive."
    }
    SYSTEM_ERROR = {
        "code": "1677929221",
        "message": "System error."
    }
    STATUS_ERROR = {
        "code": "1677929222",
        "message": "Status error."
    }
    DUPLICATE_NAME = {
        "code": "1677929223",
        "message": "The name is duplicated."
    }
    WRONG_ONLINE_STATUS = {
        "code": "1677748993",
        "message": "Wrong with object's online status"
    }
    USER_OR_PASSWORD_IS_INVALID = {
        "code": "1677929224",
        "message": "The user name or password is incorrect."
    }
    REGISTRATION_INFORMATION_IS_INCORRECT = {
        "code": "1677931281",
        "message": "Registration information is incorrect."
    }
    AGENT_VERSION_INFORMATION_IS_INCORRECT = {
        "code": "1677931281t",
        "message": "AGENT_VERSION information is incorrect."
    }
    REVERSE_REPLICATION_ERROR = {
        "code": "1677749505",
        "message": "Reverse replication does not allow replication or archiving."
    }
    CAN_NOT_PROTECT_BOTH_INSTANCE_AND_DATABASE = {
        "code": "1677932047",
        "message": "Related resource is protected."
    }
    RESOURCE_NUM_EXCEED_LIMIT = {
        "code": "1677931446",
        "message": "The total number of resources exceeds the upper limit."
    }
    CAN_NOT_PROTECT_BOTH_DATABASE_AND_TABLESPACE = {
        "code": "1677931779",
        "message": "Related resource is protected."
    }
    CANNOT_PROTECT_ARCHIVE_REPLICATION_SLA = {
        "code": "1677931536",
        "message": "worm resource can not protect replication or archive policy."
    }
    PROTECTED_COPY_OBJECT_HAS_SLA = {
        "code": "1677932039",
        "message": "Remove the copy protection of the resource and try again."
    }
    EXIST_SAME_TYPE_JOB_IN_RUNNING = {
        "code": "1677934343",
        "message": "Same scan job is running."
    }
    RESOURCE_NOT_TRUST = {
        "code": "1677931445",
        "message": "The host has not trusted"
    }
    COPY_IS_BEING_USED = {
        "code": "1593989381",
        "message": "The copy is being used."
    }
    STORAGE_PARAM_ERROR = {
        "code": "1677873251",
        "message": "Storage username or password error."
    }
    STORAGE_ACCESS_OVER_LIMIT_ERROR = {
        "code": "1677935125",
        "message": "The number of user connections to the storage device {0} has reached the upper limit."
    }
    ERROR_DEVICE_OFFLINE = {
        "code": "1593990418",
        "message": "Device status is offline."
    }
    ERROR_EXCHANGE_PROTECT_CONFLICT = {
        "code": "1677931465",
        "message": "exchange protect conflicts."
    }
    FALSE_ALARM_NOT_INIT_WORM = {
        "code": "1593990440",
        "message": "Worm clock is not init when handle false alarm."
    }
    STORAGE_DEVICE_CONNECT_ERROR = {
        "code": "1677937152",
        "message": "The storage device cannot be connected."
    }
    USER_FUNCTION_CHECK_FAILED = {
        "code": "1677947139",
        "message": "User-related functions are disabled"
    }
    RESTORE_SUB_OBJECT_NUM_MAX_LIMIT = {
        "code": "1677933076",
        "message": "Total sub object num over 1000"
    }
    RESTORE_SUB_OBJECT_MAX_BYTES = {
        "code": "1677933077",
        "message": "Total sub object size over 1024 * 1000"
    }

    INFECTED_COPY_CAN_NOT_OPERATION = {
        "code": "1677936416",
        "message": "Copy can not do the operation because of infected."
    }

    BACK_UP_NOT_SUPPORT_BASIC_DISK = {
        "code": "1677932815",
        "message": "Operation failed because the application cannot be backed up on the local disk."
    }

    NOT_A_FULL_OR_NATIVE_COPY = {
        "code": "1677932054",
        "message": "The copy is not a full copy or a native copy."
    }

    COPY_HAS_ARCHIVED_TO_STORAGE = {
        "code": "1677935131",
        "message": "The copy has archived to storage."
    }

    APPLICATION_NOT_SUPPORT_TAPE_ARCHIVE_AUTO_INDEX = {
        "code": "1677932053",
        "message": "Application not support tape archive auto index."
    }

    ARCHIVE_TO_STORAGE_JOB_EXISTED = {
        "code": "1677935132",
        "message": "Archive to storage job existed."
    }

    BOTH_SLA_WORM_AND_ANTI_RANSOMWARE_WORM_TURN_ON = {
        "code": "1677932057",
        "message": "sla worm and anti worm policy turn on cannot together exist."
    }
    MODIFY_WORM_VALIDITY_TIME_EXCEEDS_COPY_RETENTION_TIME_ERROR = {
        "code": "1677932056",
        "message": "worm expiration time can not greater than copy expiration time."
    }

    BASIC_DISK_NOT_SUPPORT_WORM_AND_ANTI = {
        "code": "1677931555",
        "message": "Operation failed because the local disk cannot support worm and antiransom."
    }