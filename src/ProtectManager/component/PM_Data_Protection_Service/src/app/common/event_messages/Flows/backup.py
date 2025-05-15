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
from typing import List

from app.common.event_messages.event import EventBase


class BackupScheduleRequest(EventBase):
    default_topic = 'schedule.backup'
    group_backup_topic = 'schedule.group_backup'

    def __init__(
            self,
            request_id,
            user_id,
            params
    ):
        super().__init__(request_id, BackupScheduleRequest.default_topic)
        self.request_id = request_id
        self.user_id = user_id
        self.params = params


class BackupInit(EventBase):
    default_topic = 'protection.backup.init'

    def __init__(
            self,
            request_id,
            user_id,
            params
    ):
        super().__init__(request_id, BackupInit.default_topic)
        self.request_id = request_id
        self.user_id = user_id
        self.params = params


class BackupDone(EventBase):
    default_topic = 'protection.backup.done'

    def __init__(self, copy_ids: List[str], request_id: str, job_id: str, status):
        EventBase.__init__(self, request_id, BackupDone.default_topic)
        self.copy_ids = copy_ids
        self.job_id = job_id
        self.status = status


class BackupComplete(EventBase):
    default_topic = 'protection.backup.completed'

    def __init__(self, request_id, related_request_id, resource_id, execute_type, sla, policy, resource,
                 current_operate_user_id=None, copy_ids=None):
        super().__init__(request_id, BackupComplete.default_topic, None)
        self.related_request_id = related_request_id
        self.resource_id = resource_id
        self.execute_type = execute_type
        self.sla = sla
        self.policy = policy
        self.resource = resource
        self.current_operate_user_id = current_operate_user_id
        self.copy_ids = copy_ids
