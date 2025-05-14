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
import ssl
from threading import Lock

import urllib3
from requests import PreparedRequest
from urllib3.response import HTTPResponse
from urllib3.util import parse_url

from app.common.constants.constant import SecurityConstants, ServiceConstants
from app.common import logger
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.security.kmc_util import Kmc
from app.common.util import file_utils


def handle_field_params(method, url, fields: dict):
    """
    URL查询参数处理：删除为None的内容|列表类型数据转换
    """
    if not fields or method not in {"DELETE", "GET", "HEAD", "OPTIONS"}:
        return url, fields
    scheme, auth, host, port, path, query, fragment = parse_url(url)
    encoded_fields = PreparedRequest._encode_params(fields)
    query = f"{query}&{encoded_fields}" if query else encoded_fields
    return f"{path}?{query}", {}


def is_response_status_ok(response: HTTPResponse):
    if response:
        return response.status < 400
    return False


def decode_response_data(response_data):
    response_content = None
    if response_data is not None:
        response_content = response_data.decode('utf-8')
    return response_content


def parse_response_data(response_data):
    if response_data:
        return json.loads(response_data.decode('utf-8'))
    return {}


def request_wrap(func):
    def inner(*args, **kwargs):
        exception_info = None
        if 'exception_info' in kwargs:
            exception_info = kwargs.get("exception_info")
            kwargs.pop('exception_info')

        response: HTTPResponse = func(*args, **kwargs)
        if not is_response_status_ok(response):
            if exception_info:
                log = logger.get_logger(__name__)
                log.error(exception_info)
            raise EmeiStorBizException.build_from_error(parse_response_data(response.data))
        return response

    return inner


def _build_ssl_context():
    context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
    context.check_hostname = False
    context.load_cert_chain(
        certfile=SecurityConstants.INTERNAL_CERT_FILE,
        keyfile=SecurityConstants.INTERNAL_KEY_FILE,
        password=Kmc().decrypt(file_utils.read_file_by_utf_8(SecurityConstants.INTERNAL_KEYFILE_PWD_FILE)),
    )
    context.load_verify_locations(cafile=SecurityConstants.INTERNAL_CA_FILE)
    context.verify_mode = ssl.CERT_REQUIRED
    return context


class InternalSslContext:
    _instance_lock = Lock()
    _instance = None
    _ssl_context = None

    def __new__(cls, *args, **kwargs):
        if cls._instance is None:
            with cls._instance_lock:
                if cls._instance is None:
                    cls._instance = object.__new__(cls)
                    cls._ssl_context = _build_ssl_context()
        return cls._instance

    @property
    def ssl_context(self):
        return self._ssl_context


class InternalHttpsClient:
    urllib3.disable_warnings(urllib3.exceptions.SecurityWarning)

    def __init__(self, host, port=None, **kwargs):
        if ":" in host:
            host = f'[{host}]'
        self.conn = urllib3.HTTPSConnectionPool(
            host,
            port=int(port) if port else None,
            ssl_context=InternalSslContext().ssl_context,
            **kwargs
        )

    def request(self, method, url, fields=None, headers=None, build_exception=False, **urlopen_kw):
        if headers is None:
            headers = {"Content-Type": "application/json"}
        try:
            url, fields = handle_field_params(method, url, fields)
            if build_exception:
                response = request_wrap(self.conn.request)(method, url, fields=fields, headers=headers, **urlopen_kw)
            else:
                response = self.conn.request(method, url, fields=fields, headers=headers, **urlopen_kw)
            return response
        except Exception as ex:
            log = logger.get_logger(__name__)
            log.exception("[InternalHttpsClient] request exception.")
            raise ex
        finally:
            self.conn.close()


class InfrastructureHttpsClient(InternalHttpsClient):
    def __init__(self, **kwargs):
        super().__init__(ServiceConstants.INFRASTRUCTURE_HOSTNAME,
                         port=ServiceConstants.INFRASTRUCTURE_PORT,
                         **kwargs)


class DataEnableEngineHttpsClient(InternalHttpsClient):
    def __init__(self, **kwargs):
        super().__init__(ServiceConstants.DATA_ENABLE_ENGINE_HOSTNAME,
                         port=ServiceConstants.DATA_ENABLE_ENGINE_PORT,
                         **kwargs)


class DataEnableEngineParserHttpsClient(InternalHttpsClient):
    def __init__(self, **kwargs):
        super().__init__(ServiceConstants.DEE_PARSER_HOSTNAME,
                         port=ServiceConstants.DEE_PARSER_PORT,
                         **kwargs)


class ProtectEngineEDmaHttpsClient(InternalHttpsClient):
    def __init__(self, **kwargs):
        super().__init__(ServiceConstants.PROTECTENGINE_E_DMA_HOSTNAME,
                         port=ServiceConstants.PROTECTENGINE_E_DMA_PORT,
                         **kwargs)


class ProtectionServiceHttpsClient(InternalHttpsClient):
    def __init__(self, **kwargs):
        super().__init__(ServiceConstants.PM_PROTECTION_SERVICE_HOSTNAME,
                         port=ServiceConstants.PM_PROTECTION_SERVICE_PORT,
                         **kwargs)


class SystemBaseHttpsClient(InternalHttpsClient):
    def __init__(self, **kwargs):
        super().__init__(ServiceConstants.PM_SYSTEM_BASE_HOSTNAME,
                         port=ServiceConstants.PM_SYSTEM_BASE_PORT,
                         **kwargs)


class UbcHttpsClient(InternalHttpsClient):
    def __init__(self, **kwargs):
        super().__init__(ServiceConstants.UBC_HOSTNAME,
                         port=ServiceConstants.UBC_PORT,
                         **kwargs)


class OsaHttpsClient(InternalHttpsClient):
    def __init__(self, **kwargs):
        super().__init__(ServiceConstants.OSA_HOSTNAME,
                         port=ServiceConstants.OSA_PORT,
                         **kwargs)
