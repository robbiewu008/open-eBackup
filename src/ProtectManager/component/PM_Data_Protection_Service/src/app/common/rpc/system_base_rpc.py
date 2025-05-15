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

from fastapi import HTTPException

from app.common.clients.client_util import SystemBaseHttpsClient, parse_response_data, is_response_status_ok
from app.common.logger import get_logger

log = get_logger(__name__)


def encrypt(plaintext):
    params = {
        "plaintext": plaintext
    }
    response = SystemBaseHttpsClient().request(
        "POST", f'/v1/kms/encrypt', body=json.dumps(params))
    if not is_response_status_ok(response):
        raise HTTPException(response.status)
    return parse_response_data(response.data)['ciphertext']


def decrypt(ciphertext):
    params = {
        "ciphertext": ciphertext
    }
    response = SystemBaseHttpsClient().request(
        "POST", f'/v1/kms/decrypt', body=json.dumps(params))
    if not is_response_status_ok(response):
        raise HTTPException(response.status)
    return parse_response_data(response.data)['plaintext']


def get_host_trust(agent_ip):
    params = {"endpoint": agent_ip}
    response = SystemBaseHttpsClient().request(
        "GET", f"/v2/internal/environments/host/trust", fields=params)
    if not is_response_status_ok(response):
        raise HTTPException(response.status)
    return parse_response_data(response.data).get("isTrusted")


def delete_host_ubackup_agent(host_id: str):
    # 删除外置通用代理
    response = SystemBaseHttpsClient().request(
        "delete", f"/v2/internal/environments/delete/{host_id}")

    log.debug(f"post delete host: {parse_response_data(response.data)}")
    if not is_response_status_ok(response):
        raise HTTPException(response.status)
