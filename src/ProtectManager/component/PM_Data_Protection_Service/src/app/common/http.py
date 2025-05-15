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
from typing import TypeVar

import requests
from requests import Response, Session
from requests.adapters import HTTPAdapter, DEFAULT_POOLSIZE, DEFAULT_POOLBLOCK, Retry

from app.common import logger
from app.common.exception.unified_exception import EmeiStorBizException

T = TypeVar('T')
LOGGER = logger.get_logger(__name__)


def wrap(method: T) -> T:
    """
    request method wrapper, return a wrapped method, which can check response error info

    :param method: request method
    :return: wrapped method
    """

    def wrapper(*args, **kwargs):
        exception_info = None
        if 'exception_info' in kwargs:
            exception_info = kwargs.get("exception_info")
            kwargs.pop('exception_info')
        response: Response = method(*args, **kwargs)
        if not response.ok:
            if exception_info:
                LOGGER.info(exception_info)
            raise EmeiStorBizException.build_from_error(response.json())
        return response

    return wrapper


DEFAULT_STATUS_FORCE_LIST = frozenset([413, 429, 500, 502, 503, 504])
SHORT_RETRY_POLICY = Retry(
    connect=10,
    total=10,
    backoff_factor=0.1,
    status_forcelist=DEFAULT_STATUS_FORCE_LIST
)
LONG_RETRY_POLICY = Retry(
    connect=10000,
    total=10000,
    backoff_factor=1,
    status_forcelist=DEFAULT_STATUS_FORCE_LIST
)


def adapt(
        pool_connections=DEFAULT_POOLSIZE,
        pool_maxsize=DEFAULT_POOLSIZE,
        max_retries=SHORT_RETRY_POLICY,
        pool_block=DEFAULT_POOLBLOCK
):
    session = requests.Session()
    adapter = HTTPAdapter(
        pool_connections=pool_connections,
        pool_maxsize=pool_maxsize,
        max_retries=max_retries,
        pool_block=pool_block
    )
    session.mount('http://', adapter)

    class Wrapper:
        def __init__(self):
            self.methods = {}

        def __getattribute__(self, item):
            if item == 'methods':
                return object.__getattribute__(self, item)
            if item not in ["get", "post", "delete", "put", "request"]:
                return getattr(session, item)
            methods: dict = object.__getattribute__(self, 'methods')
            method = methods.get(item)
            if method is not None:
                return method
            method = wrap(getattr(session, item))
            method.__name__ = item
            methods[item] = method
            return method

    wrapper: Session = Wrapper()
    return wrapper


get = wrap(requests.get)
post = wrap(requests.post)
delete = wrap(requests.delete)
put = wrap(requests.put)
request = wrap(requests.request)
