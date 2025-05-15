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
from app.resource.models.rbac_models import DomainResourceObjectTable
from app.common.logger import get_logger

log = get_logger(__name__)


def get_domain_id_list(resource_object_id: str, domain_id: str = None):
    with database.session() as session:
        domain_list = session.query(DomainResourceObjectTable.domain_id).filter(
            DomainResourceObjectTable.resource_object_id == resource_object_id).all()
        domain_id_list = [domain_resource.domain_id for domain_resource in domain_list]
        if domain_id and domain_id not in domain_id_list:
            domain_id_list.append(domain_id)
        return domain_id_list


def get_domain_id_list_by_resource_object_list(resource_object_id_list: [str]):
    with database.session() as session:
        domain_list = session.query(DomainResourceObjectTable.domain_id).filter(
            DomainResourceObjectTable.resource_object_id.in_(resource_object_id_list)).all()
        domain_id_list = [domain_resource.domain_id for domain_resource in domain_list]
        return domain_id_list


def get_domain_resource_object_relation(domain_id: str, resource_object_id: str, resource_type_list: [str] = None):
    with database.session() as session:
        query_relation = session.query(DomainResourceObjectTable) \
            .filter(DomainResourceObjectTable.domain_id == domain_id) \
            .filter(DomainResourceObjectTable.resource_object_id == resource_object_id)
        if resource_type_list:
            query_relation = query_relation.filter(DomainResourceObjectTable.type.in_(resource_type_list))
        return query_relation.first()


def delete_domain_resource_object_relation(domain_id=None, resource_object_id=None, resource_type=None):
    with database.session() as session:
        relation = session.query(DomainResourceObjectTable)
        if domain_id:
            relation = relation.filter(DomainResourceObjectTable.domain_id == domain_id)
        if resource_object_id:
            relation = relation.filter(DomainResourceObjectTable.resource_object_id == resource_object_id)
        if resource_type:
            relation = relation.filter(DomainResourceObjectTable.type == resource_type)
        relation = relation.delete()
        session.flush()
