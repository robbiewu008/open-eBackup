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
SERVICE_NAME = "rlm"
DEFAULT_PRIORITY = 3

MESSAGE_OK = 'success'
MESSAGE_FAIL = 'fail'

RESOURCE_ID = 'id'
LOCK_TYPE = 'lock_type'

LOCK = "lock"
UNLOCK = "unlock"
QUERY = "query"

READ_LOCK = "r"
WRITE_LOCK = "w"

REQUEST_API = "api"
REQUEST_ID = "request_id"
REQUEST_RESPONSE = "response_topic"

STATE_PENDING = "PENDING"
STATE_LOCKED = "LOCKED"
STATE_UNLOCK = "UNLOCK"

PROGRESS_START = "running"
PROGRESS_SUCCEEDED = "succeeded"
PROGRESS_FAILED = "failed"

REDIS_RESOURCE_LOCK_PREFIX = 'redis_resource_lock_'


class GaussdbCertConfig:
    # gaussdb cert config
    CONN_ARGS = {
        "sslmode": "verify-ca",
        "sslrootcert": "/opt/OceanProtect/infrastructure/cert/internal/ca/ca.crt.pem"
    }
    AUTOCOMMIT = "AUTOCOMMIT"
