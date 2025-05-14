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
from app.common.event_messages.event import EventBase, ElasticSearchDocument


class StartDiscoveryRequest(EventBase):

    default_topic = 'StartDiscoveryRequest'

    def __init__(self, request_id, env_name, env_type, user_name,
                 password, ip, ports, rescan_interval_in_sec, response_topic=''):
        super().__init__(request_id, StartDiscoveryRequest.default_topic, response_topic)
        self.env_name = env_name
        self.env_type = env_type
        self.user_name = user_name
        self.password = password
        self.ip = ip
        self.ports = ports
        self.rescan_interval_in_sec = rescan_interval_in_sec


class InsertDiscoveryOwnRequest(ElasticSearchDocument):

    default_topic = 'env_discovery'

    def __init__(self, request_id, es_doc):
        super().__init__(request_id, InsertDiscoveryOwnRequest.default_topic, es_doc=es_doc)


class InsertDiscoveryGSRequest(ElasticSearchDocument):

    default_topic = 'gl_resources'

    def __init__(self, request_id, es_doc):
        super().__init__(request_id, InsertDiscoveryGSRequest.default_topic, es_doc=es_doc)
