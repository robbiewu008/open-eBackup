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
from app.resource.models.resource_models import ResourceTable, ResourceDesesitizationTable
from app.base.db_base import database


def query_resource_info(resource_id):
    with database.session() as session:
        resource_info: ResourceTable = session.query(ResourceTable).filter(
            ResourceTable.uuid == resource_id).one_or_none()
        return resource_info


def query_desesitization_info(resource_id):
    with database.session() as session:
        desestitation_info: ResourceDesesitizationTable = session.query(ResourceDesesitizationTable).filter(
            ResourceDesesitizationTable.uuid == resource_id).one_or_none()
        return desestitation_info


def create_desesitization_info(desesitization_info):
    with database.session() as db:
        db.add(desesitization_info)


def update_desesitization_info(desesitization_info):
    with database.session() as db:
        filters = [ResourceDesesitizationTable.uuid == desesitization_info.uuid]
        db.query(ResourceDesesitizationTable).filter(*filters)\
            .update({ResourceDesesitizationTable.identification_status: desesitization_info.identification_status,
                     ResourceDesesitizationTable.desesitization_status: desesitization_info.desesitization_status,
                     ResourceDesesitizationTable.identification_job_id: desesitization_info.identification_job_id,
                     ResourceDesesitizationTable.desesitization_job_id: desesitization_info.desesitization_job_id,
                     ResourceDesesitizationTable.desesitization_policy_id:
                                 desesitization_info.desesitization_policy_id,
                     ResourceDesesitizationTable.desesitization_policy_name:
                     desesitization_info.desesitization_policy_name})
