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

class Error(Exception):
    """Raised when something failed in an unexpected and unrecoverable way"""
    pass


class OperationError(Error):
    """Raised when an operation failed in an expected but unrecoverable way"""
    pass


class NotifyError(Error):
    """Raised when an notify operation failed in an expected but unrecoverable way"""
    pass


class DBConnectionError(OperationError):
    """Raised when a db connection error occurs"""
    pass


class DBAuthenticationError(OperationError):
    """Raised when a db authentication error occurs"""
    pass


class DBOperationError(OperationError):
    """Raised when a db operation error occurs"""
    pass


class DBAuthorizedError(OperationError):
    """Raised when a db authorized error occurs"""
    pass
