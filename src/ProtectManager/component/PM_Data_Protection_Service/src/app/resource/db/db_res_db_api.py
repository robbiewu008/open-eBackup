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
from app.resource.models.resource_models import EnvironmentTable
from app.common.logger import get_logger

log = get_logger(__name__)


def query_host_info_by_uuid(host_id):
    with database.session() as session:
        environment: EnvironmentTable = session.query(EnvironmentTable).filter(
            EnvironmentTable.uuid == host_id).one_or_none()
        return environment


def query_host_in_cluster(session, filters):
    host_endpoint_under_the_cluster = []
    items = session.query(EnvironmentTable). \
        filter(*filters). \
        distinct(EnvironmentTable.uuid). \
        order_by(EnvironmentTable.uuid).all()
    for item in items:
        if item is not None:
            host_endpoint = item.endpoint.replace("{", "").replace("}", "")
            host_endpoint_under_the_cluster.extend(host_endpoint.split(','))
    return host_endpoint_under_the_cluster


def escape_str(values: str):
    for token in '#%?*_':
        values = values.replace(token, f"#{token}")
    return values


def build_is_cluster(condition, filter_list):
    if len(list(condition.get("is_cluster"))) == 1:
        is_cluster = True if str(condition.get("is_cluster")[0]).lower() == "true" else False
        filter_list.append(EnvironmentTable.is_cluster == is_cluster)
        if not is_cluster:
            filter_list.append(EnvironmentTable.parent_uuid == EnvironmentTable.root_uuid)
