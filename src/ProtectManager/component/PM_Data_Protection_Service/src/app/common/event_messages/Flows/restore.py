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


class StartVmRestoreRequest(EventBase):

    default_topic = 'StartVmRestoreRequest'

    def __init__(self, request_id, object_type, env_type, es_user_id, snap_id, src_restore_paths,
                 dest_info, post_restore_config):

        super().__init__(request_id, StartVmRestoreRequest.default_topic)
        self.object_type = object_type
        self.env_type = env_type
        self.es_user_id = es_user_id
        self.snap_id = snap_id
        self.src_restore_paths = src_restore_paths
        self.dest_info = dest_info
        self.post_restore_config = post_restore_config


class StartFileSetRestoreRequest(EventBase):

    default_topic = 'StartFileSetRestoreRequest'

    def __init__(self, request_id, object_type, env_type, es_user_id, snap_id, src_restore_paths,
                 dest_info, replace_mode):

        super().__init__(request_id, StartFileSetRestoreRequest.default_topic)
        self.object_type = object_type
        self.env_type = env_type
        self.es_user_id = es_user_id
        self.snap_id = snap_id
        self.src_restore_paths = src_restore_paths
        self.dest_info = dest_info
        self.replace_mode = replace_mode


class StartDBRestoreRequest(EventBase):

    default_topic = 'StartDBRestoreRequest'

    def __init__(self, request_id, object_type, env_type, es_user_id, snap_id, original_database,
                 time_point, scn, dest_info, restore_config):

        super().__init__(request_id, StartDBRestoreRequest.default_topic)
        self.object_type = object_type
        self.env_type = env_type
        self.es_user_id = es_user_id
        self.snap_id = snap_id
        self.original_database = original_database
        self.time_point = time_point
        self.scn = scn
        self.dest_info = dest_info
        self.restore_config = restore_config


class RestoreDone(EventBase):
    default_topic = 'protection.restore.done'

    def __init__(self, request_id: str, job_id: str):
        EventBase.__init__(self, request_id, RestoreDone.default_topic)
        self.job_id = job_id
