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


class CopyErrorCode(BaseErrorCode):
    FORBID_DELETE_LATEST_COPY = {
        "code": "1677933313",
        "message": "Not allow delete the latest copy."
    }
    ALREADY_IN_DELETING = {
        "code": "1677933314",
        "message": "The copy is deleting."
    }
    ALREADY_IN_DELETING_OCEAN_CYBER = {
        "code": "1677931341",
        "message": "The snapshot is deleting."
    }
    FORBID_DELETE_INDEXING_COPY = {
        "code": "1677933315",
        "message": "Not allow delete the indexing copy."
    }
    ERROR_INCREMENT_COPY_EXPIRATION_TIME = {
        "code": "1677933319",
        "message": "Not allow modify increment copy expiration time."
    }
    ERROR_FULL_COPY_EXPIRATION_TIME = {
        "code": "1677933320",
        "message": "Not allow modify full copy expiration time."
    }
    COPY_RETENTION_TYPE_RETAIN_PERMANENT = {
        "code": "1677933321",
        "message": "The retention type of copy should be permanent."
    }
    ERROR_UPDATE_COPY_EXPIRE_TIME = {
        "code": "1677933322",
        "message": "Copy expire time is earlier than system current time."
    }
    ERROR_UPDATE_SNAPSHOT_EXPIRE_TIME = {
        "code": "1677933341",
        "message": "Snapshot expire time is earlier than system current time."
    }
    ERROR_DELETE_COPY_EXIST_CLONE_FILE_SYSTEM = {
        "code": "1593988934",
        "message": "The copy has a clone file system."
    }
    COPY_NOT_EXIST = {
        "code": "1677933323",
        "message": "The copy is not exists."
    }
    ANTI_RANSOMWARE_REPORT_NOT_EXIST = {
        "code": "1677933324",
        "message": "The detection report does not exist."
    }
    COPY_NOT_INFECTED = {
        "code": "1677933327",
        "message": "The copy is not in the infected state."
    }
    SNAPSHOT_NOT_INFECTED = {
        "code": "1677933342",
        "message": "The snapshot is not in the infected state."
    }
    COPY_IS_DETECTING = {
        "code": "1677933328",
        "message": "The copy is under ransomware detection."
    }
    SNAPSHOT_IS_DETECTING = {
        "code": "1677933343",
        "message": "The snapshot is under ransomware detection."
    }
    FORBID_DELETE_LATEST_COPY_LINK = {
        "code": "1677933329",
        "message": "Not allow delete the latest copy link."
    }
    FORBID_DUPLICATE_COPY_NAME = {
        "code": "1677933332",
        "message": "Copy name cannot duplicate."
    }
    FORBID_DUPLICATE_SNAPSHOT_NAME = {
        "code": "1677933344",
        "message": "Snapshot name cannot duplicate."
    }
    COPY_IS_SECURITY_SNAP = {
        "code": "1677933338",
        "message": "The copy is security snap."
    }
    MODIFY_WORM_COPY_RETENTION_FAIL = {
        "errorCode": "1677936395",
        "errorMessage": "The retention of worm copy can only be increased.",
        "parameters": []
    }
    MODIFY_SECURITY_SNAP_RETENTION_FAIL = {
        "errorCode": "1677936394",
        "errorMessage": "The retention of snapshot security can only be increased.",
        "parameters": []
    }
    DELETE_WORM_COPY_FAIL = {
        "errorCode": "1677936396",
        "errorMessage": "Worm copy can not be deleted manually.",
        "parameters": []
    }
    DELETE_WORM_RELATED_COPY_FAIL = {
        "code": "1677936406",
        "message": "the related copy is worm.",
        "parameters": []
    }
    CAN_NOT_MODIFY_COPY_RETENTION = {
        "errorCode": "1677933317",
        "errorMessage": "Retention of copy whose worm status is setting can not modify",
        "parameters": []
    }
    WORM_INNER_DIRECTORY_RETENTION_MODIFY = {
        "errorCode": "1677933339",
        "errorMessage": "the retention of worm copy whose format is inner directory can not modify.",
        "parameters": []
    }
    EXIST_COPY_DELETE_JOB = {
        "code": "1677933318",
        "message": "The copy exist copy delete or expire job."
    }
    ERROR_DELETE_BROWSING_COPY = {
        "code": "1677933349",
        "message": "The copy is being browsed."
    }