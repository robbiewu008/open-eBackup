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

import re
from functools import cache

from openGauss.common.common import get_hostname, check_path, check_injection_char
from openGauss.resource.resources import GaussCluster


class ResourceInfo:
    def __init__(self, user_name, env_file=""):
        self._cluster = GaussCluster(user_name, env_file)
        if not self._cluster.nodes:
            self._cluster.get_cluster_nodes()

    def get_node_status(self, endpoint):
        for node in self._cluster.nodes:
            if node.node_ip == endpoint:
                return node.instance_state
        return ""

    def get_deploy_type(self):
        return self._cluster.deploy_type

    def get_sync_state(self):
        return self._cluster.sync_state

    def get_instance_data_path(self):
        data_path = self._cluster.get_instance_data_path()
        return data_path if check_path(data_path) else ""

    def get_cluster_nodes(self):
        nodes = [node.node_ip for node in self._cluster.nodes]
        return nodes

    def get_local_endpoint(self):
        hostname = get_hostname()
        for node in self._cluster.nodes:
            if node.node_name == hostname:
                return node.node_ip
        return ""

    def get_instance_port(self):
        port = self._cluster.get_instance_port()
        return port if isinstance(port, str) and check_injection_char(port) else ""

    def get_node_role(self, endpoint):
        for node in self._cluster.nodes:
            if node.node_ip == endpoint:
                return node.instance_role
        return ""

    def get_node_role_caps(self, endpoint):
        ret, output = self._cluster.get_cluster_nodes_detail()
        if not ret:
            return ""
        for line in output.split('\n'):
            if line.find(endpoint) > -1:
                pat = re.compile("\s[PS]\s")
                put = pat.search(line).group(0).strip()
                return put
        return ""

    def get_db_version(self):
        return self._cluster.db_version

    def get_node_receiver_replay_location(self, endpoint):
        receiver_replay_location = None
        for node in self._cluster.nodes:
            if node.node_ip == endpoint:
                receiver_replay_location = node.receiver_replay_location
        return receiver_replay_location

    def get_cluster_status(self):
        return self._cluster.cluster_state

    @cache
    def get_local_cn_port(self):
        return self._cluster.get_local_cn_port()
