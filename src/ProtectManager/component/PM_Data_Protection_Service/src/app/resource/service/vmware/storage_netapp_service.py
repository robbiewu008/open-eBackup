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
import base64
import http
import json

import urllib3
from requests import Session

from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.logger import get_logger
from app.resource.common.constants import DmaProxy

log = get_logger(__name__)


class StorageNetAppService:
    def __init__(self, storage_ip: str, port: 443, username: str, password: str):
        urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)
        self.storage_ip = storage_ip
        self.session = Session()
        self.session.verify = False
        self.session.proxies.update({
            'https': "http://" + DmaProxy.host + ":" + str(DmaProxy.port),
            'http': "http://" + DmaProxy.host + ":" + str(DmaProxy.port)
        })
        self.session.auth = (username.encode(), password.encode())
        self.username = username
        self.password = password
        self._connect()

    def is_remote_host_name_in_netapp_storage_ip_address(self, remote_host_name):
        res = self.session.get(f'https://{self.storage_ip}/api/network/ip/interfaces?fields=ip.address,svm', timeout=20)
        if res.status_code == http.HTTPStatus.OK:
            data = json.loads(res.text)
            records = data.get('records', {})
            for record in records:
                ip_address = record.get('ip').get('address')
                if remote_host_name == ip_address:
                    log.info(f"Remote_host_name: {remote_host_name} is in NetApp: {self.storage_ip}, ip: {ip_address}")
                    return True
        return False

    def _connect(self):
        authorization = base64.b64encode((self.username + ":" + self.password).encode('utf-8'))
        self.session.headers.update({'Authorization': 'Basic ' + str(authorization)})
        res = self.session.get(f'https://{self.storage_ip}/api/cluster', timeout=20)
        if res.status_code == http.HTTPStatus.OK:
            data = json.loads(res.text)
            management_interfaces = data.get('management_interfaces', {})
            management_ip = management_interfaces[0].get('ip').get('address')
        else:
            log.error(f"Failed to login NetApp storage: {self.storage_ip}.")
            raise EmeiStorBizException(CommonErrorCodes.STORAGE_PARAM_ERROR, message="Login storage failed.")
        self.session.headers.update({'Content-Type': 'application/json;charset=UTF-8'})
