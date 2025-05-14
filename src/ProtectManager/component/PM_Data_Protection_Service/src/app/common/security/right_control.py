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

from fastapi import Header
from fastapi import Request

from app.common.constants.constant import RBACConstants
from app.common.constraints import thread_local
from app.common.deploy_type import DeployType
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exception.user_error_codes import UserErrorCodes
from app.common.log.kernel import call_origin_method_with_record_log
from app.common.security.jwt_utils import auth_rule_check, get_arguments_as_dict, auth_resource_check, \
    check_user_auth, resolve_target, get_user_info_from_token
from app.common.toolkit import Method


def right_control(roles=None, resources=None, check_defined_role=False, check_auth=None):
    def decorator(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            token = kwargs.get('token')
            request: Request = kwargs.get('fast_api_request')
            user_info = get_user_info_from_token(token)
            user_default_role = user_info.get('role-list')[0]
            if check_auth and not DeployType().is_not_support_rbac_deploy_type():
                if not token:
                    raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)
                check_auth_resources = resolve_target(check_auth.target, args, kwargs)
                domain_id = user_info.get('domain-id')
                check_user_auth(domain_id, check_auth.resource_set_type, check_auth.operation,
                                check_auth.auth_operation_list, check_auth_resources)
            if DeployType().is_not_support_rbac_deploy_type() or check_defined_role \
                    or user_default_role in RBACConstants.DEFAULT_IN_ROLE_NAME_LIST:
                check(args, kwargs, token, request)
            thread_local.right_control = token
            try:
                operation_config = getattr(thread_local, 'operation_config', None)
                if operation_config is None:
                    return method(*args, **kwargs)
                else:
                    return call_origin_method_with_record_log(method, token, operation_config, *args, **kwargs)
            finally:
                del thread_local.right_control

        def check(args, kwargs, token, request):
            if not token:
                raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)
            if roles is not None:
                auth_rule_check(token, roles, request)
            if resources is not None:
                arguments = get_arguments_as_dict(args, kwargs)
                auth_resource_check(token, resources, arguments)

        method = Method(func)
        method.append_parameter("token", default=Header(
            ...,
            alias="X-Auth-Token",
            title="X-Auth-Token",
            description="访问令牌"
        ))
        method.append_parameter("fast_api_request", annotation=Request)
        wrapper.__signature__ = method.signature
        return wrapper

    return decorator
