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
from app.resource.models.rbac_models import ResourceSetResourceObjectTable


def delete_resource_set_resource_relation(resource_set_id=None, resource_object_id=None, resource_type=None):
    with database.session() as session:
        relation = session.query(ResourceSetResourceObjectTable)
        if resource_set_id:
            relation = relation.filter(ResourceSetResourceObjectTable.resource_set_id == resource_set_id)
        if resource_object_id:
            relation = relation.filter(ResourceSetResourceObjectTable.resource_object_id == resource_object_id)
        if resource_type:
            relation = relation.filter(ResourceSetResourceObjectTable.type == resource_type)
        relation.delete()
        session.flush()
