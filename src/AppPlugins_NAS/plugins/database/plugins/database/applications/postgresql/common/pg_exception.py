#
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
#

import json


class ErrCodeException(Exception):
    def __init__(self, err_code, *parameters, message: str = None):
        super(ErrCodeException, self).__init__(message)
        self._error_code = err_code
        self._parameters = parameters
        self._message = message

    @property
    def error_code(self):
        return self._error_code

    @property
    def parameter_list(self):
        return [param for param in self._parameters] if self._parameters else list()

    @property
    def error_message(self):
        return json.dumps({"errorCode": self._error_code, "parameters": self._parameters,
                           "errorMessage": self._message})
