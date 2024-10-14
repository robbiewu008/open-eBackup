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

from fastapi.encoders import jsonable_encoder

from public_cbb.exception.error_codes import BaseErrorCode


class CustomException(Exception):
    def __init__(self, err: BaseErrorCode, *args):
        super().__init__()
        self._error_code = err['code']
        self._error_msg = err['message']
        self._params = args

    @property
    def error_code(self):
        return self._error_code

    @property
    def parameter_list(self):
        return [param for param in self._params] if self._params else []

    @classmethod
    def build_from_error(cls, error: dict):
        """
        将异常信息转换成 CustomException

        :param error: error info
        :return: BizException
        """
        error_code = error.get('errorCode')
        message = error.get('errorMessage')
        params = error.get('detailParams')
        params = params if isinstance(params, (list, tuple, set)) else [params] if params is not None else []
        return cls(dict(code=error_code, message=message), *params)

    def get_error_rsp(self):
        if self._params:
            # 为不影响原本公共函数的返回，加特殊逻辑。PM模块解析时错误码参数键为parameters
            return jsonable_encoder({"errorCode": self._error_code, "errorMessage": self._error_msg,
                                     "parameters": self._params})
        return jsonable_encoder({"errorCode": self._error_code, "errorMessage": self._error_msg,
                                 "detailParams": self._params})
