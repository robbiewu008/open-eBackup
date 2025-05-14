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
from app.common.event_messages.event import EventBase, EventResponseBase


class DeleteSnapRequest(EventBase):

    default_topic = 'DeleteSnapRequest'

    def __init__(self, request_id, snap_id, chain_id=None, timestamp=None,
                 es_user_id=None, policy_obj=None, protected_obj=None, response_topic=''):
        super().__init__(request_id, DeleteSnapRequest.default_topic, response_topic)
        self.snap_id = snap_id
        self.chain_id = chain_id
        self.timestamp = timestamp
        self.es_user_id = es_user_id
        self.poicy_obj = policy_obj
        self.protected_obj = protected_obj


class DeleteSnapResponse(EventResponseBase):

    default_topic = 'DeleteSnapResponse'

    def __init__(self, request_id, status="success", error_desc=''):
        super().__init__(request_id, DeleteSnapResponse.default_topic, status, error_desc)


class DeleteSnapCleanRequest(EventBase):

    default_topic = 'DeleteSnapCleanRequest'

    def __init__(self, request_id, response_topic=''):
        super().__init__(request_id, DeleteSnapCleanRequest.default_topic, response_topic)
