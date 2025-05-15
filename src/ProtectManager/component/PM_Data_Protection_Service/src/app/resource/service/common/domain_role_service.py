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
from app.base.db_base import database
from app.resource.models.rbac_models import DomainRoleTable, RoleTable, RoleAuthTable, AuthTable, ResourceSetTable, \
    ResourceSetResourceObjectTable


def get_resource_set_auth_list(domain_id: str, auth_operation_list: list[str], target: str,
                               resource_type: str):
    with database.session() as session:
        domain_role_list = session.query(DomainRoleTable) \
            .join(RoleTable, RoleTable.role_id == DomainRoleTable.role_id) \
            .join(RoleAuthTable, RoleAuthTable.role_id == RoleTable.role_id) \
            .join(AuthTable, AuthTable.uuid == RoleAuthTable.role_auth_id) \
            .join(ResourceSetTable, ResourceSetTable.uuid == DomainRoleTable.resource_set_id) \
            .join(ResourceSetResourceObjectTable,
                  ResourceSetResourceObjectTable.resource_set_id == ResourceSetTable.uuid) \
            .filter(DomainRoleTable.domain_id == domain_id) \
            .filter(AuthTable.auth_operation.in_(auth_operation_list)) \
            .filter(ResourceSetResourceObjectTable.resource_object_id == target) \
            .filter(ResourceSetResourceObjectTable.type == resource_type).all()
        resource_set_id_list = [domain_role.resource_set_id for domain_role in domain_role_list]
        return resource_set_id_list
