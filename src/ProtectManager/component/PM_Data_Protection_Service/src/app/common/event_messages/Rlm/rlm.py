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


class LockRequest(EventBase):

    default_topic = 'LockRequest'

    def __init__(self, request_id, resources, wait_timeout, lock_id, priority=4, response_topic=''):
        super().__init__(request_id, LockRequest.default_topic, response_topic)
        self.resources = resources
        self.wait_timeout = wait_timeout
        self.lock_id = lock_id
        self.priority = priority


class LockResponse(EventResponseBase):

    default_topic = 'LockResponse'

    def __init__(self, request_id, status="success", error_desc=''):
        super().__init__(request_id, LockResponse.default_topic, status, error_desc)


class UnlockRequest(EventBase):

    default_topic = 'UnlockRequest'

    def __init__(self, request_id, lock_id, response_topic=''):
        super().__init__(request_id, UnlockRequest.default_topic, response_topic)
        self.lock_id = lock_id


class UnlockResponse(EventResponseBase):

    default_topic = 'UnlockResponse'

    def __init__(self, request_id, status="success", error_desc=''):
        super().__init__(request_id, UnlockResponse.default_topic, status, error_desc)


class HasLockRequest(EventBase):

    default_topic = 'HasLockRequest'

    def __init__(self, request_id, lock_id):
        super().__init__(request_id, HasLockRequest.default_topic)
        self.lock_id = lock_id


class HasLockResponse(EventResponseBase):

    default_topic = 'HasLockResponse'

    def __init__(self, request_id, status: bool):
        super().__init__(request_id, HasLockResponse.default_topic, str(status), '')
