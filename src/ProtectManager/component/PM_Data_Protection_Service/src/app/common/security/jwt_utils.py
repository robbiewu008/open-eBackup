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
from typing import List, Callable, Iterable

from cryptography.hazmat.backends import default_backend
from cryptography.x509 import load_pem_x509_certificate
import jwt

from app.common import logger
from app.common.clients.client_util import ProtectionServiceHttpsClient, is_response_status_ok, SystemBaseHttpsClient
from app.common.clients.system_base_client import SystemBaseClient
from app.common.constants.constant import RBACConstants
from app.common.enums.rbac_enum import ResourceSetTypeEnum, OperationTypeEnum, AuthOperationEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exception.user_error_codes import UserErrorCodes
from app.common.security.role_dict import RoleEnum
from app.common.security.user_type_dict import UserTypeEnum
from app.common.toolkit import get_value_by_path
from app.resource.service.common.domain_resource_object_service import get_domain_resource_object_relation
from app.resource.service.common.domain_role_service import get_resource_set_auth_list
from app.resource.service.common.role_auth_service import get_default_role_auth_list
from app.resource.service.common.role_service import get_default_role_list

GLOBAL_PUBLIC_KEY = None

log = logger.get_logger(__name__)


def get_user_info_from_token(token):
    global GLOBAL_PUBLIC_KEY
    if not GLOBAL_PUBLIC_KEY:
        data = SystemBaseClient.get_auth_certificate()
        certificate = load_pem_x509_certificate(str.encode(data), default_backend())
        GLOBAL_PUBLIC_KEY = certificate.public_key()
    token_parsed = jwt.decode(token, key=GLOBAL_PUBLIC_KEY, verify=True, algorithms=["RS256"])
    role_name_list = []
    if 'user' in token_parsed and token_parsed['user'] is not None:
        if "domainId" in token_parsed['user'] and token_parsed['user']['domainId'] is not None:
            role_name_list = [role.role_name for role in get_default_role_list(token_parsed['user']['domainId'])]
    else:
        raise EmeiStorBizException(error=UserErrorCodes.ACCESS_DENIED, parameters=[])
    return {
        'user-name': token_parsed['user']['name'],
        'user-id': token_parsed['user']['id'],
        'domain-id': token_parsed['user']['domainId'],
        'user-type': token_parsed['user']['userType'],
        'is-hcs-user-manage-permission': token_parsed['user']['hcsUserManagePermission'],
        'role-list': role_name_list,
        'es-valid-token': "true",
    }


def get_username(token):
    return get_user_info_from_token(token).get('user-name')


def get_user_id(token):
    return get_user_id_from_user_info(get_user_info_from_token(token))


def get_user_domain_id(token):
    return get_user_domain_id_from_user_info(get_user_info_from_token(token))


def get_user_id_from_user_info(user_info: dict):
    return user_info.get('user-id')


def get_user_domain_id_from_user_info(user_info: dict):
    return user_info.get('domain-id')


def get_role_list(token):
    return get_role_list_from_user_info(get_user_info_from_token(token))


def get_role_list_from_user_info(user_info: dict):
    return user_info.get('role-list')


def auth_rule_check(token, role_list, request):
    user_info = get_user_info_from_token(token)
    is_hcs_user_manager_permission = user_info.get('is-hcs-user-manage-permission')
    user_type = user_info.get('user-type')
    method = request.method
    if UserTypeEnum.HCS.value == user_type:
        # 如果是HCS用户但是没有管理权限，只能调用GET方法，否则报错
        if not is_hcs_user_manager_permission and method.upper() != 'GET':
            raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)
    roles = user_info.get('role-list')
    for role in roles:
        if RoleEnum.is_builtin_role_name(role) and RoleEnum(role) in role_list:
            return
    user_name = user_info.get('user-name')
    log.error(f'Current user: {user_name} has no permission.')
    raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)


def check_user_is_admin_or_audit(user_info):
    return check_user_role(user_info, RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_AUDITOR, mode=any)


def check_user_is_admin(user_info):
    return check_user_role(user_info, RoleEnum.ROLE_SYS_ADMIN, mode=any)


def check_user_role(user_info, *expect_role_list, mode: Callable[[Iterable[object]], bool]):
    role_list = get_role_list_from_user_info(user_info)
    if not role_list:
        return False
    role_list = [RoleEnum(role) for role in role_list if RoleEnum.is_builtin_role_name(role)]
    return mode(role in role_list for role in expect_role_list)


def auth_resource_check(token, resources, arguments):
    user_info = get_user_info_from_token(token)
    if check_user_is_admin_or_audit(user_info):
        return
    user_id = get_user_id_from_user_info(user_info)
    resolved_resources = resolve_resources(resources, arguments)
    for name, resource_list in resolved_resources.items():
        checker = RESOURCE_OWNERSHIP_CHECKERS[name]
        if checker is None:
            raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)
        if resource_list:
            checker(user_id, resource_list)


def check_protected_copy_ownership(user_id: str, resource_uuid_list: List[str]):
    check_ownership(f"/v1/internal/protected-copy-objects/action/verify",
                    params={"user_id": user_id, "resource_uuid_list": resource_uuid_list})


def check_resource_ownership(user_id: str, resource_uuid_list: List[str]):
    check_ownership(f"/v1/internal/resource/action/verify",
                    params={"user_id": user_id, "resource_uuid_list": resource_uuid_list})


def check_copy_ownership(user_id: str, copy_uuid_list: List[str]):
    check_ownership(f"/v1/internal/copies/action/verify",
                    params={"user_id": user_id, "copy_uuid_list": copy_uuid_list})


def check_qos_ownership(user_id: str, uuid_list: List[str]):
    check_ownership(f"/v1/internal/qos/action/verify",
                    params={"user_id": user_id, "qos_uuid_list": uuid_list})


def check_sla_ownership(user_id: str, uuid_list: List[str]):
    response = SystemBaseHttpsClient().request("GET", f"/v1/internal/sla/action/verify",
                                               fields={"user_id": user_id, "sla_uuid_list": uuid_list})
    if not is_response_status_ok(response):
        if response.status == 500:
            raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)
        else:
            raise EmeiStorBizException(CommonErrorCodes.SYSTEM_ERROR)


def check_ownership(ms_url: str, params):
    response = ProtectionServiceHttpsClient().request("GET", ms_url, fields=params)
    if not is_response_status_ok(response):
        if response.status == 500:
            raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)
        else:
            raise EmeiStorBizException(CommonErrorCodes.SYSTEM_ERROR)


RESOURCE_OWNERSHIP_CHECKERS = {
    "resource": check_resource_ownership,
    "copy": check_copy_ownership,
    "sla": check_sla_ownership,
    "qos": check_qos_ownership,
    "protected_copy": check_protected_copy_ownership,
}


def resolve_resources(resources, arguments):
    resources = resources if isinstance(resources, (list, tuple)) else [resources]
    resources = [resource for resource in resources if resource and ':' in resource]
    results = dict()
    for resource in resources:
        index = resource.find(':')
        name = resource[0:index]
        start = index + 1
        items = [item for item in resource[start:].split(',') if item]
        result = []
        for item in items:
            expr = item.strip()
            value = get_value_by_path(arguments, expr)
            value = flat(value)
            result.extend(value)
        array = results.get(name, [])
        array = array + [str(item) for item in result if item]
        results[name] = array
    return results


def flat(obj):
    results = []
    if isinstance(obj, (tuple, list, set)):
        for item in obj:
            results.extend(flat(item))
    elif obj is not None:
        results.append(obj)
    return results


def get_arguments_as_dict(args, kwargs):
    arguments = {}
    for index, value in enumerate(args):
        arguments[str(index)] = value
    for name, value in kwargs.items():
        arguments[name] = value
    return arguments


def resolve_target(target, args, kwargs):
    results = []
    if not target:
        return results
    arguments = get_arguments_as_dict(args, kwargs)
    resources = [resource for resource in target.split(',') if resource]
    for resource in resources:
        expr = resource.strip()
        value = get_value_by_path(arguments, expr)
        if isinstance(value, (list, tuple)):
            results = results + [str(val) for val in value]
            continue
        results = results + [str(value)]
    return results


def check_user_auth(domain_id: str, resource_set_type: ResourceSetTypeEnum, operation: OperationTypeEnum,
                    auth_operation_list: List[AuthOperationEnum], resources: List[str]):
    auth_operation_value_list = []
    if auth_operation_list:
        auth_operation_value_list = [auth_operation.value for auth_operation in auth_operation_list if auth_operation]
    if operation == OperationTypeEnum.CREATE:
        create_operation_validate(domain_id, auth_operation_value_list)
    elif operation == OperationTypeEnum.DELETE or operation == OperationTypeEnum.MODIFY:
        delete_or_modify_operation_validate(domain_id, resource_set_type.value, auth_operation_value_list, resources)
    elif operation == OperationTypeEnum.QUERY:
        query_operation_validate(domain_id, resource_set_type.value, resources)


def create_operation_validate(domain_id: str, auth_operation_list: List[str]):
    # 查询用户默认角色是否有创建权限
    if not get_default_role_auth_list(domain_id, auth_operation_list):
        log.error(f'Current user has no create permission.')
        raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)


def delete_or_modify_operation_validate(domain_id: str, resource_set_type: str, auth_operation_list: List[str],
                                        resources: list[str]):
    if not resources:
        return
    for resource_object_id in resources:
        # 判断该资源是否在当前用户域内
        actual_resource_set_type = get_actual_resource_set_type(resource_set_type, domain_id, resource_object_id)

        # 只分域不分权资源类型 不校验删除或修改权限
        if actual_resource_set_type in RBACConstants.ONLY_IN_DOMAIN_RESOURCE_TYPE_LIST:
            continue
        # 分域分权资源类型 校验是否有删除权限
        if not get_resource_set_auth_list(domain_id, auth_operation_list, resource_object_id,
                                          actual_resource_set_type):
            log.error(f'Current user has no modify or delete permission of resource: {resource_object_id}')
            raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)


def query_operation_validate(domain_id: str, resource_set_type: str, resources: list[str]):
    if not resources:
        return
    for resource_object_id in resources:
        # 判断该资源是否在当前用户域内
        get_actual_resource_set_type(resource_set_type, domain_id, resource_object_id)


def get_actual_resource_set_type(resource_set_type: str, domain_id: str, resource_object_id: str):
    if resource_set_type in RBACConstants.REQUIRED_TYPE_LIST:
        if not get_domain_resource_object_relation(domain_id, resource_object_id, [resource_set_type]):
            log.error(f'Resource: {resource_object_id} not in user domain: {domain_id}.')
            raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)
        return resource_set_type
    relation = get_domain_resource_object_relation(domain_id=domain_id, resource_object_id=resource_object_id)
    if not relation:
        log.error(f'Resource: {resource_object_id} not in user domain: {domain_id}.')
        raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)
    return relation.type
