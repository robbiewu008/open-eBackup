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
#!/usr/bin/env python
# _*_ coding:utf-8 _*_
import datetime
import json
import os
import socket
import ssl
import time
from enum import Enum

import OpenSSL
import urllib3
from starlette import status
from urllib3.exceptions import HTTPError

from public_cbb.device_manager.constants import CRL_CONTENT_INDEX
from public_cbb.log.logger import get_logger
from public_cbb.security.anonym_utils.anonymity import Anonymity
from public_cbb.security.pwd_manage.pwd_manage import clear
from public_cbb.security.pwd_manage.cert_manage import CertManage
from public_cbb.config.global_config import get_settings
from public_cbb.utils.decorator import retry

logger = get_logger()


class CertSource(str, Enum):
    INTERNAL = 'internal'
    EXTERNAL = 'external'


class HttpRequest:
    def __init__(self):
        self.method = ''
        self.host = ''
        self.port = ''
        self.suffix = ''
        self.body = None
        self.headers = {}
        self.cookie = None
        self.https = False
        self.cert_source = CertSource.INTERNAL
        self.verify = get_settings().OPEN_CERT_VERIFY


def get_src_logical_ip(destination_ip, port):
    req = HttpRequest()
    req.method = 'GET'
    req.suffix = f'/v1/internal/deviceManager/rest/logical_ip'
    req.host = os.getenv('NODE_IP')
    req.port = '30173'
    if ':' in destination_ip:
        destination_ip = destination_ip.strip("[]")
    req.body = json.dumps({
        "task_type": "backup",
        "destination_ip": destination_ip,
        "port": str(port)
    })
    req.https = True
    request = RequestUtils(req)
    status_code, rsp = request.send_request()
    if status_code == status.HTTP_200_OK and rsp.get("error", {}).get('code', 1) == 0:
        logger.info(f'Get src logical ip success, destination ip: {destination_ip}, src ip: {rsp.get("logical_ip")}')
        return rsp.get("logical_ip")
    else:
        return ""


class RequestUtils(object):
    def __init__(self, req: HttpRequest):
        self.req = req
        self.verify = False if not self.req.https else self.req.verify
        self.rsp = None
        self.key_pass = ''

    @staticmethod
    def _is_ipv6(ip):
        try:
            socket.inet_pton(socket.AF_INET6, ip)
        except socket.error:
            return False
        return True

    @staticmethod
    def _get_http_instance_with_no_verify(req: HttpRequest):
        settings = get_settings()
        if req.https:
            http = urllib3.HTTPSConnectionPool(req.host, req.port, maxsize=settings.MAX_CACHE_CONNECTION,
                                               retries=settings.MAX_HTTP_RETRIES, timeout=settings.HTTP_TIME_OUT,
                                               cert_reqs='CERT_NONE', assert_hostname=False)
        else:
            http = urllib3.HTTPConnectionPool(req.host, req.port, maxsize=settings.MAX_CACHE_CONNECTION,
                                              retries=settings.MAX_HTTP_RETRIES, timeout=settings.HTTP_TIME_OUT)
        return http

    @staticmethod
    def _is_json(string):
        try:
            json.loads(string)
        except ValueError:
            return False
        return True

    @classmethod
    @retry(HTTPError, get_settings().MAX_HTTP_RETRIES, get_settings().HTTP_TIME_OUT)
    def send_request_with_crl_check(cls, req: HttpRequest, context, bind_address=False):
        http_status = None
        data = None
        settings = get_settings()
        try:
            if bind_address:
                vrf_bind_option = (socket.SOL_SOCKET, socket.SO_BINDTODEVICE, "vrf-srv".encode('utf-8'))
                connect = urllib3.HTTPSConnectionPool(
                    req.host, req.port, maxsize=settings.MAX_CACHE_CONNECTION,
                    retries=settings.MAX_HTTP_RETRIES, timeout=settings.HTTP_TIME_OUT, ssl_context=context,
                    socket_options=[vrf_bind_option], source_address=(get_src_logical_ip(req.host, req.port), 0),
                    assert_hostname=False
                )
            else:
                connect = urllib3.HTTPSConnectionPool(req.host, req.port, maxsize=settings.MAX_CACHE_CONNECTION,
                                                      retries=settings.MAX_HTTP_RETRIES, timeout=settings.HTTP_TIME_OUT,
                                                      ssl_context=context, assert_hostname=False)
            response = connect.request(req.method, req.suffix, headers=req.headers, body=req.body)
        except Exception as e:
            logger.error(f'Send request with crl failed, e:{Anonymity.process(str(e))}')
            return http_status, data
        if not response:
            return http_status, data
        http_status, rsp_data = response.status, response.data.decode('utf-8')
        data = json.loads(rsp_data) if cls._is_json(rsp_data) else rsp_data
        return http_status, data

    @classmethod
    def send_request_with_crl_check_no_retry(cls, req: HttpRequest, context, bind_address=False):
        http_status = None
        data = None
        settings = get_settings()
        if bind_address:
            vrf_bind_option = (socket.SOL_SOCKET, socket.SO_BINDTODEVICE, "vrf-srv".encode('utf-8'))
            connect = urllib3.HTTPSConnectionPool(
                req.host, req.port, maxsize=settings.MAX_CACHE_CONNECTION,
                retries=0, timeout=settings.HTTP_TIME_OUT, ssl_context=context,
                socket_options=[vrf_bind_option], source_address=(get_src_logical_ip(req.host, req.port), 0),
                assert_hostname=False
            )
        else:
            connect = urllib3.HTTPSConnectionPool(req.host, req.port, maxsize=settings.MAX_CACHE_CONNECTION,
                                                  retries=0, timeout=settings.HTTP_TIME_OUT,
                                                  ssl_context=context, assert_hostname=False)
        response = connect.request(req.method, req.suffix, headers=req.headers, body=req.body)
        if not response:
            return http_status, data
        http_status, rsp_data = response.status, response.data.decode('utf-8')
        data = json.loads(rsp_data) if cls._is_json(rsp_data) else rsp_data
        return http_status, data

    @classmethod
    def get_ssl_context(cls, ca_dir, cert_dir, key_dir, crl_path):
        try:
            logger.debug(f'Start get ssl context...')
            ssl_context = ssl.SSLContext(protocol=ssl.PROTOCOL_TLSv1_2)
            ssl_context.verify_mode = ssl.CERT_REQUIRED
            ssl_context.check_hostname = False
            key_pass = CertManage.get_cert_pwd(get_settings().EXTERNAL_CNF_DIR)
            ssl_context.load_cert_chain(cert_dir, key_dir, key_pass)
            # 吊销列表校验
            if os.path.exists(crl_path) and not cls._crl_is_expired(crl_path):
                logger.debug(f'Set crl_path...')
                ssl_context.verify_flags = ssl.VERIFY_CRL_CHECK_CHAIN
                ssl_context.load_verify_locations(crl_path)
            else:
                logger.debug(f'No need load crl, path not exist or expired')
            ssl_context.load_verify_locations(ca_dir)
            return ssl_context
        except Exception as e:
            logger.error(f'Get ssl context failed, e:{Anonymity.process(str(e))}')
            return ''

    @classmethod
    def _crl_is_expired(cls, crl_path):
        with open(crl_path, 'rb') as _crl_file:
            crl = _crl_file.read()
            cert = OpenSSL.crypto.load_crl(OpenSSL.crypto.FILETYPE_PEM, crl)
            dump_crl = OpenSSL.crypto.dump_crl(OpenSSL.crypto.FILETYPE_TEXT, cert)
            crl_text = dump_crl.decode("UTF-8")
            for line in crl_text.split("\n"):
                if "Next Update: " not in line:
                    continue
                key, value = line.split(":", CRL_CONTENT_INDEX)
                expire_date = datetime.datetime.strptime(value.strip(), "%b %d %X %Y %Z")
                expire_time = int(time.mktime(expire_date.timetuple()))
                current_time = int(time.time())
                if expire_time <= current_time:
                    logger.info(f'Crl has expired, expire time:{expire_time}, current time:{current_time}')
                    return True
                break
        logger.debug(f'Crl not expired')
        return False

    def send_request(self):
        if self.verify:
            # 开启证书认证
            http = self._get_http_instance_with_verify(self.req)
        else:
            http = self._get_http_instance_with_no_verify(self.req)

        if self.req.cookie:
            self.req.headers['Cookie'] = self.req.cookie
        logger.info(f'Method: {self.req.method} Url: {self._get_url_from_http_req(self.req)}')
        try:
            self.rsp = http.request(self.req.method, self.req.suffix, headers=self.req.headers, body=self.req.body)
        except HTTPError as ex:
            logger.error('Fail to send request!', exc_info=True)
            raise ex
        finally:
            clear(self.key_pass)

        rsp_data = self.rsp.data.decode('utf8')
        data = json.loads(rsp_data) if self._is_json(rsp_data) else rsp_data
        return self.rsp.status, data

    def get_headers(self):
        return self.rsp.headers if self.rsp else None

    def _get_url_from_http_req(self, req):
        if self._is_ipv6(req.host):
            url = f"[{req.host}]:{req.port}{req.suffix}"
        else:
            url = f"{req.host}:{req.port}{req.suffix}"
        if req.https:
            return f"https://{url}"
        else:
            return f"http://{url}"

    def _get_http_instance_with_verify(self, req: HttpRequest):
        settings = get_settings()
        cert_file = settings.INTERNAL_CERT_DIR if req.cert_source == CertSource.INTERNAL \
            else settings.EXTERNAL_CERT_DIR
        key_file = settings.INTERNAL_KEY_DIR if req.cert_source == CertSource.INTERNAL else settings.EXTERNAL_KEY_DIR
        ca_certs = settings.INTERNAL_CA_DIR if req.cert_source == CertSource.INTERNAL else settings.EXTERNAL_CA_DIR
        cnf_file = settings.INTERNAL_CNF_DIR if req.cert_source == CertSource.INTERNAL else settings.EXTERNAL_CNF_DIR
        self.key_pass = CertManage.get_cert_pwd(cnf_file)
        http = urllib3.HTTPSConnectionPool(req.host, req.port, maxsize=settings.MAX_CACHE_CONNECTION,
                                           retries=settings.MAX_HTTP_RETRIES, timeout=settings.HTTP_TIME_OUT,
                                           cert_reqs='CERT_REQUIRED', cert_file=cert_file, key_file=key_file,
                                           key_password=self.key_pass, ca_certs=ca_certs, assert_hostname=False)
        return http
