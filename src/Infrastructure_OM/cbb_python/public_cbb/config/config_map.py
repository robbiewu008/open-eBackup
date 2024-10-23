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

import time
from fastapi import status
from urllib3.exceptions import HTTPError

from public_cbb.communication.rest.https_request import HttpRequest, RequestUtils
from public_cbb.config.global_config import get_settings
from public_cbb.log.logger import get_logger

log = get_logger()


def get_general_user(init=False):
    return get_user('database.generalUsername', 'database.generalPassword', init)


def get_super_user(init=False):
    return get_user('database.superUsername', 'database.superPassword', init)


def get_user(user_key, pass_key, init=False):
    ret, config_map = get_common_secret(init)
    if not ret:
        msg = f'Can not get common secret.'
        raise Exception(msg)
    user_name = user_pass = None
    try:
        for data in config_map.get('data'):
            if user_key in data:
                user_name = data.get(user_key)
            if pass_key in data:
                user_pass = data.get(pass_key)
        if not user_name or not user_pass:
            msg = "Can't find database user in common secret."
            raise Exception(msg)
        return user_name, user_pass
    except Exception as e:
        msg = f'Get general user info failed!, {str(e)}'
        raise Exception(msg) from e


def get_common_conf():
    req = HttpRequest()
    settings = get_settings()
    req.host, req.port = settings.INFRA_HOST, settings.INFRA_HTTP_PORT
    req.method = 'GET'
    req.suffix = '/v1/infra/configmap/info?nameSpace=dpa&configMap=common-conf'
    req.https = True
    try:
        request = RequestUtils(req)
        status_code, data = request.send_request()
        if status_code == status.HTTP_200_OK and not data.get('error'):
            return True, data
        else:
            return False, dict()
    except HTTPError:
        return False, dict()


def get_common_secret(init=False):
    settings = get_settings()
    retry_times = settings.INFRA_HTTP_RETRY_TIMES
    while True:
        try:
            req = HttpRequest()
            req.host, req.port = settings.INFRA_HOST, settings.INFRA_HTTP_PORT
            req.method = 'GET'
            req.suffix = '/v1/infra/secret/info?nameSpace=dpa&secretName=common-secret'
            req.https = True
            request = RequestUtils(req)
            status_code, data = request.send_request()
            if status_code == status.HTTP_200_OK and not data.get('error'):
                return True, data
            else:
                return False, dict()
        except HTTPError:
            if not init and retry_times == 0:
                log.error(f'Get common secret failed, no more retry.')
                return False, dict()
            retry_times = retry_times - 1 if not init else retry_times
            log.exception(f'Get common secret failed, waiting retry...')
            time.sleep(settings.INFRA_HTTP_RETRY_INTERVAL)
