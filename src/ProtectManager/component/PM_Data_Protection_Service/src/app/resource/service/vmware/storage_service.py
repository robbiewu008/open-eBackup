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
from ipaddress import ip_address, IPv6Address

import urllib3
from requests import Session

from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.logger import get_logger
from app.resource.common.constants import DmaProxy

log = get_logger(__name__)

# 校验登录是否成功的错误码，为0则登陆成功，默认值设为-1
ERROR_CODE = -1


class StorageService:
    def __init__(self, storage_ip: str, username: str, password: str, port=8088):
        urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)
        self.host = 'https://' + self.format_ip(storage_ip) + ":" + str(port)
        self.session = Session()
        self.session.verify = False
        self.session.proxies.update({
            'https': "http://" + DmaProxy.host + ":" + str(DmaProxy.port),
            'http': "http://" + DmaProxy.host + ":" + str(DmaProxy.port)
        })
        self.session.auth = (username.encode(), password.encode())
        self.username = username
        self.password = password
        self._get_esn()
        self._fill_session_header()

    @staticmethod
    def format_ip(storage_ip):
        try:
            connect_ip = ip_address(storage_ip)
        except ValueError as e:
            raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM,
                                       message="The storage ip isn't either a v4 or a v6 address.") from e
        if isinstance(connect_ip, IPv6Address):
            connect_ip = '[' + str(connect_ip) + ']'
        return str(connect_ip)

    def get_free_effective_capacity_from_storage(self):
        res = self.session.get(f'{self.host}/deviceManager/rest/{self.esn}/effective_capacity_info')
        free_effective_capacity = json.loads(res.text).get('data', {}).get("freeEffectiveCapacity", '')
        # 从注册存储中获取存储的剩余容量，容量需要除以扇区数2，得到的容量单位为kb，保留3位小数
        free_effective_capacity = format(int(free_effective_capacity) / 2, '.3f')
        return json.loads(free_effective_capacity)

    def is_wwn_in_storage(self, wwn):
        res = self.session.get(f'{self.host}/deviceManager/rest/{self.esn}/lun?filter=WWN::{wwn}')
        if 'data' in res.text:
            log.info(f"Wwn: {wwn} is in storage: {self.host}")
            return True
        return False

    def is_remote_host_in_storage(self, remote_host):
        res = self.session.get(f'{self.host}/deviceManager/rest/{self.esn}/lif?filter=IPV4ADDR:{remote_host}')
        if 'data' in res.text:
            log.info(f"Remote host: {remote_host} is in storage: {self.host}")
            return True
        return False

    def is_storage_service_valid(self):
        res = self.session.get(f'{self.host}/deviceManager/rest/{self.esn}/check_ibase_token')
        if not res.text:
            return False
        error = json.loads(res.text).get('error', {})
        if error.get('code', ERROR_CODE) == 0:
            return True
        log.error(f"Storage service is invalid. Error is: {error}")
        return False

    def _get_esn(self):
        kwargs = {
            "username": self.username,
            "password": self.password,
            "scope": "0",
            "loginMode": "4",
            "isEncrypt": False
        }
        try:
            res = self.session.post(f'{self.host}/deviceManager/rest//login', json=kwargs)
        except Exception as e:
            log.error(f"Error occurred when login storage: {self.host}.")
            raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM, message="Login storage failed!") from e
        # 检查用户名密码是否正确
        error = json.loads(res.text).get('error', {})
        log.info(f"Error param in login storage response is: {error}, storage: {self.host}.")
        if error.get('code', ERROR_CODE) != 0:
            if (str(error.get('code')) == "1077949067"):
                raise EmeiStorBizException(CommonErrorCodes.STORAGE_ACCESS_OVER_LIMIT_ERROR,
                                           self.host.split(':')[1][2:],
                                           message="The number of user connections to the storage device has reached "
                                                   "the upper limit.")
            if str(error.get('code')) == "-1007":
                raise EmeiStorBizException(CommonErrorCodes.STORAGE_ACCESS_OVER_LIMIT_ERROR,
                                           self.host.split(':')[1][2:],
                                           message="The verification code is not entered when login storage.")
            else:
                raise EmeiStorBizException(CommonErrorCodes.STORAGE_PARAM_ERROR, message="Login storage failed.")
        data = json.loads(res.text).get('data', {})
        self.esn = data.get("deviceid", '')
        self.ibase_token = data.get('iBaseToken', '')

    def _fill_session_header(self):
        headers = {'Content-Type': 'application/json;charset=UTF-8'}
        self.session.headers = headers
        self.session.headers.update({'iBaseToken': self.ibase_token})
