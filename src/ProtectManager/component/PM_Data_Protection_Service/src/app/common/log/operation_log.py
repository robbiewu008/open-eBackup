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
from functools import wraps

from starlette.requests import Request

from app.common.constraints import thread_local
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.log.event_schemas import OperationConfig
from app.common.log.kernel import call_origin_method_with_record_log, data_loaders
from app.common.toolkit import Method, listify


def define_data_loader(name, loader):
    if name in data_loaders:
        raise EmeiStorBizException(
            CommonErrorCodes.SYSTEM_ERROR,
            message=f"define data loader conflict."
        )
    data_loaders[name] = loader


def operation_log(name, target, detail=()):
    detail = detail if callable(detail) else listify(detail)
    if isinstance(name, int):
        operation_id = hex(name)
        operation_id = operation_id[:2] + operation_id[2:].upper()
    else:
        operation_id = str(name)
    operation_config = OperationConfig(
        name=operation_id,
        target=target,
        detail=detail,
    )

    def decorator(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            token = kwargs.get('token') if 'token' in kwargs else getattr(thread_local, 'right_control', None)
            return call_origin_method_with_record_log(method, token, operation_config, *args, **kwargs)

        method = Method(func)
        method.append_parameter("fast_api_request", annotation=Request)
        wrapper.__signature__ = method.signature
        return wrapper

    return decorator
