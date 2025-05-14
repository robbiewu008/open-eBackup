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


class ProgressUpdate(EventBase):

    default_topic = "ProgressUpdate"

    def __init__(self, request_id, timestamp, status, operation, progress, details):

        super().__init__(request_id, ProgressUpdate.default_topic)
        self.timestamp = timestamp
        self.status = status
        self.operation = operation
        self.progress = progress
        self.details = details


class ProgressUpdateAdditionalInfo(EventBase):

    default_topic = "ProgressUpdateAdditionalInfo"

    def __init__(
        self,
        request_id,
        time_span=None,
        actual_data_amount=None,
        original_data_amount=None,
        snapshot_info=None,
    ):

        super().__init__(request_id, ProgressUpdateAdditionalInfo.default_topic)
        self.time_span = time_span
        self.actual_data_amount = actual_data_amount
        self.original_data_amount = original_data_amount
        self.snapshot_info = snapshot_info


class InsertProgressDetailsRequest(ElasticSearchDocument):

    default_topic = "progress_reports"

    def __init__(self, request_id, es_doc):
        super().__init__(
            request_id, InsertProgressDetailsRequest.default_topic, es_doc=es_doc
        )
