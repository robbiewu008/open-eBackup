#
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
#

from common.const import RoleType
from openGauss.common.const import CLUSTER_FIELD, INNER_DB, NODE_FIELD
from openGauss.resource.database import Database


class ClusterNode:
    _filed_map = dict(zip(NODE_FIELD, CLUSTER_FIELD))

    def __init__(self):
        self.node_ip = None
        self.instance_role = None
        self.instance_port = None
        self.instance_state = None
        self.node_name = None
        self.instance_id = None
        self.data_path = None
        self.db_user = None
        self.db_env_path = None
        self.receiver_replay_location = ""

    @property
    def role_type(self):
        role_type = getattr(self, "instance_role", "")
        if role_type == "Primary":
            return RoleType.PRIMARY
        if role_type == "Standby":
            return RoleType.STANDBY
        return RoleType.NONE_TYPE

    def to_cluster_dict(self):
        cluster_dict = {
            field: getattr(self, field, "")
            for field in CLUSTER_FIELD
        }
        return cluster_dict

    def set_field_value(self, field, value):
        if field not in CLUSTER_FIELD:
            field = self._filed_map.get(field)
        if field in CLUSTER_FIELD:
            setattr(self, field, value)

    def get_databases(self, db_user, db_env_path):
        database = Database(db_user=db_user, db_port=self.instance_port, db_env_path=db_env_path)
        all_databases = [db for db in database.show_databases() if db not in INNER_DB]
        return all_databases
