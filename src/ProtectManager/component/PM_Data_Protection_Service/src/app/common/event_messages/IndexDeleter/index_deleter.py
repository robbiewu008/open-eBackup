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


class IndexDeleteRequest(EventBase):

    default_topic = 'IndexDeleteRequest'

    def __init__(self, request_id, snap_id, response_topic=''):
        super().__init__(request_id, IndexDeleteRequest.default_topic, response_topic)
        self.snap_id = snap_id


class IndexDeleteResponse(EventResponseBase):

    default_topic = 'IndexDeleteResponse'

    def __init__(self, request_id, status="success", error_desc=''):
        super().__init__(request_id, IndexDeleteResponse.default_topic, status, error_desc)
