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
from app.common import logger
from app.common.deploy_type import DeployType
from app.common.enums.rbac_enum import ResourceSetTypeEnum
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exception.user_error_codes import UserErrorCodes
from app.copy_catalog.service.curd.copy_query_service import query_user_related_copy_by_resource_id
from app.resource.service.common.domain_resource_object_service import get_domain_resource_object_relation
from app.resource.service.common.domain_role_service import get_resource_set_auth_list

log = logger.get_logger(__name__)


def check_copy_in_domain(copy_id=None, domain_id=None):
    if not get_domain_resource_object_relation(domain_id, copy_id, [ResourceSetTypeEnum.COPY]):
        log.error(f'Copy : {copy_id} not in current user domain: {domain_id}')
        raise EmeiStorBizException(error=UserErrorCodes.ACCESS_DENIED)


def check_resource_related_copy_auth(resource_id=None, auth_operation_list=None, domain_id=None, generated_by=None):
    if DeployType().is_not_support_rbac_deploy_type() or not resource_id:
        return
    if not domain_id:
        raise EmeiStorBizException(error=UserErrorCodes.ACCESS_DENIED)
    related_copy_id_list = query_user_related_copy_by_resource_id(resource_id=resource_id, domain_id=domain_id,
                                                                  generated_by=generated_by)
    if not related_copy_id_list:
        log.info(f'No copy exists in user domain: {domain_id}')
        raise EmeiStorBizException(error=UserErrorCodes.ACCESS_DENIED)
    for copy_id in related_copy_id_list:
        check_copy_operation_auth(copy_id, auth_operation_list, domain_id)


def check_copy_query_auth(copy_id=None, domain_id=None):
    if DeployType().is_not_support_rbac_deploy_type() or not copy_id:
        return
    if not domain_id:
        raise EmeiStorBizException(error=UserErrorCodes.ACCESS_DENIED)
    check_copy_in_domain(copy_id, domain_id)


def check_copy_operation_auth(copy_id=None, auth_operation_list=None, domain_id=None):
    if DeployType().is_not_support_rbac_deploy_type() or not copy_id:
        return
    if not domain_id:
        raise EmeiStorBizException(error=UserErrorCodes.ACCESS_DENIED)
    check_copy_in_domain(copy_id, domain_id)
    if not get_resource_set_auth_list(domain_id, auth_operation_list, copy_id, ResourceSetTypeEnum.COPY):
        log.error(f'Current user has no operation permission of copy: {copy_id}')
        raise EmeiStorBizException(error=UserErrorCodes.ACCESS_DENIED)