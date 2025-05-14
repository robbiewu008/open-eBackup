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
import json

from app.common.clients.client_util import parse_response_data, SystemBaseHttpsClient
from app.common.exter_attack import exter_attack
from app.common.http import LONG_RETRY_POLICY
from app.common.logger import get_logger

log = get_logger(__name__)


def hdfs_kerberos_login(env_id, params: dict):
    params["env_id"] = env_id
    url = f"/v1/internal/resources/login"
    response = SystemBaseHttpsClient(retries=LONG_RETRY_POLICY).request("POST", url, body=json.dumps(params))
    return parse_response_data(response.data)


@exter_attack
def query_kerberos_by_id(kerberos_id):
    url = f"/v1/internal/kerberos/{kerberos_id}"
    response = SystemBaseHttpsClient(retries=LONG_RETRY_POLICY).request("GET", url)
    return parse_response_data(response.data)


def hdfs_kerberos_remove(env_id, sub_type: str):
    params = {"sub_type": sub_type}
    url = f"/v1/internal/resources/{env_id}/remove"
    response = SystemBaseHttpsClient(retries=LONG_RETRY_POLICY).request("DELETE", url, fields=params)
    return parse_response_data(response.data)
