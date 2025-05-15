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
from typing import List

from app.base.db_base import database
from app.resource.models.resource_models import ResourceCatalogTable


def list_resource_catalog():
    with database.session() as session:
        parent_catalogs = session.query(ResourceCatalogTable).filter(ResourceCatalogTable.parent_id == "-1").all()
        results = list(item.as_dict() for item in parent_catalogs)
        for item in results:
            children_catalogs = session.query(ResourceCatalogTable) \
                .filter(ResourceCatalogTable.parent_id == item["catalog_id"]).all()
            item["children"] = list(catalog.as_dict() for catalog in children_catalogs)
        return results


def hidden_catalog(catalog_ids: List[str]):
    _update_hidden_field(catalog_ids, False)


def show_catalog(catalog_ids: List[str]):
    _update_hidden_field(catalog_ids, True)


def _update_hidden_field(catalog_ids: List[str], hidden: bool):
    with database.session() as session:
        session.query(ResourceCatalogTable).filter(ResourceCatalogTable.catalog_id.in_(catalog_ids)) \
            .update({ResourceCatalogTable.show: hidden}, synchronize_session=False)
