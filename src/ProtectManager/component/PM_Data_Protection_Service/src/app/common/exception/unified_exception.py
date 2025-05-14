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
from typing import List

from fastapi.encoders import jsonable_encoder

from app.common.exception.common_error_codes import BaseErrorCode


class EmeiStorBizException(Exception):
    def __init__(self, error: BaseErrorCode, *parameters, message: str = None, retryable=False, **kwargs):
        self._error_code = error["code"]
        self._parameters = parameters
        self._retryable = retryable
        message = message if message else kwargs.get("error_message")
        message = message if message else error["message"]
        self._error_message = message

    @property
    def error_code(self):
        return self._error_code

    @property
    def parameter_list(self):
        return [parameter for parameter in self._parameters] if self._parameters else []

    @classmethod
    def build_from_error(cls, error: dict):
        """
        将异常信息转换成EmeiStorBizException

        :param error: error info
        :return: EmeiStorBizException
        """
        error_code = error['errorCode']
        message = error['errorMessage']
        params = error['parameters']
        params = params if isinstance(params, (list, tuple, set)) else [params] if params is not None else []
        return cls(dict(code=error_code, message=message), *params, message=message)

    def get_error_response(self):
        return jsonable_encoder({"errorCode": self._error_code, "errorMessage": self._error_message,
                                 "parameters": self._parameters, "retryable": self._retryable})


class IllegalParamException(Exception):
    def __init__(self, error: BaseErrorCode, parameters: List, message: str = None):
        self._error_code = error["code"]
        self._parameters = parameters
        self._error_message = message if message else error["message"]

    @property
    def error_code(self):
        return self._error_code

    def get_error_response(self):
        return jsonable_encoder({"errorCode": self._error_code, "errorMessage": self._error_message,
                                 "parameters": self._parameters})


class DBRetryException(Exception):
    pass
