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


class UserErrorCodes(BaseErrorCode):
    ACCESS_DENIED = {
        "code": "1677929497",
        "message": "Access denied."
    }
    RESOURCE_BEEN_AUTHORIZED = {
        "code": "1677747459",
        "message": "The resource has been authorized."
    }
    ALREADY_BIND_SLA = {
        "code": "1677931448",
        "message": "Resource already bind SLA."
    }
    NOT_ALLOW_AUTHORIZE_OR_REVOKE = {
        "code": "1677931288",
        "message": "Resource is not allow authorize or revoke."
    }
    RESOURCE_BEEN_OCCUPIED = {
        "code": "1677931454",
        "message": "Resource is already occupied by another resource."
    }
    ALREADY_BIND_FILESET_TEMPLATE = {
        "code": "1677931384",
        "message": "Resource already bind fileset template."
    }
    SLA_POLICY_OPERATION_ACCESS_DENIED = {
        "code": "1677752070",
        "message": "SLA policy operation no permission."
    }
