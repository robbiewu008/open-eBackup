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
from app.resource.models.rbac_models import AuthTable, RoleAuthTable, RoleTable, DomainRoleTable


def get_default_role_auth_list(domain_id: str, auth_operation_list: list[str]):
    with database.session() as session:
        auth_list = session.query(AuthTable) \
            .join(RoleAuthTable, RoleAuthTable.role_auth_id == AuthTable.uuid) \
            .join(RoleTable, RoleTable.role_id == RoleAuthTable.role_id) \
            .join(DomainRoleTable, DomainRoleTable.role_id == RoleTable.role_id) \
            .filter(DomainRoleTable.domain_id == domain_id) \
            .filter(DomainRoleTable.is_default) \
            .filter(AuthTable.auth_operation.in_(auth_operation_list)).all()
        return auth_list
