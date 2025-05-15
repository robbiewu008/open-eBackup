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
from app.common.event_messages.event import EventBase


class ResourceAddedRequest(EventBase):
    default_topic = 'resource.added'

    def __init__(
            self,
            request_id,
            resource_id
    ):
        super().__init__(request_id, ResourceAddedRequest.default_topic)
        self.request_id = request_id
        self.resource_id = resource_id


class ResourceDeletedRequest(EventBase):
    default_topic = 'resource.deleted'

    def __init__(
            self,
            request_id,
            resource_id
    ):
        super().__init__(request_id, ResourceDeletedRequest.default_topic)
        self.request_id = request_id
        self.resource_id = resource_id
