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
from datetime import datetime
from pydantic import Field
from app.common.event_messages.event import EventBase, EventMessage


class AbortRequest(EventBase):
    '''
    Abort request with timestamp in utc seconds since 1970
    '''
    default_topic = 'AbortRequest'

    def __init__(self, request_id):
        super().__init__(request_id, AbortRequest.default_topic)
        self.timestamp = datetime.utcnow().timestamp()


class AbortRequestEvent(EventMessage):
    default_publish_topic: str = 'AbortRequest'
    timestamp: float = Field(..., gt=0, description='abort request utc timestamp')
