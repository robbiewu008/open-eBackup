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
from app.resource.models.resource_models import ResourceTable
from app.common.logger import get_logger

log = get_logger(__name__)


def query_name_by_resource_ids(resource_uuids):
    if not isinstance(resource_uuids, list):
        resource_uuids = [resource_uuids]
    with database.session() as session:
        query = session.query(ResourceTable.name).filter(ResourceTable.uuid.in_(resource_uuids)).all()
        if query is not None:
            data = list(data.name.strip() for data in query)
            return_str = ", ".join(data)
            log.debug(f"[operation resource]: name : {return_str}")
            return return_str
        else:
            return "--"


def query_name_by_namespace_id(params):
    resource_uuids = params.get("namespace_id")
    return query_name_by_resource_ids(resource_uuids)


def query_name_by_dataset_uuids(params):
    delete_dataset_request = params.get("delete_dataset_request")
    dataset_uuids_dict = delete_dataset_request.__dict__
    dataset_uuids = dataset_uuids_dict.get("dataset_uuids")
    log.debug(f"[Query Resource]: name by dataset_uuids: {dataset_uuids}")
    return query_name_by_resource_ids(dataset_uuids)
