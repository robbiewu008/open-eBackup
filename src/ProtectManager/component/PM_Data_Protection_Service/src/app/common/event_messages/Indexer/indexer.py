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
from app.common.event_messages.event import EventBase, EventResponseBase, ElasticSearchDocument


class IndexRequest(EventBase):
    default_topic = 'IndexRequest'

    def __init__(self, request_id, snap_id, path, response_topic=''):
        super().__init__(request_id, IndexRequest.default_topic, response_topic)
        self.snap_id = snap_id
        self.path = path


class GenIndex(EventBase):
    default_topic = 'GenIndex'

    def __init__(self, request_id, copy_id):
        super().__init__(request_id, GenIndex.default_topic)
        self.copy_id = copy_id


class IndexResponse(EventResponseBase):
    default_topic = 'IndexResponse'

    def __init__(self, request_id, status="success", error_desc=''):
        super().__init__(request_id, IndexResponse.default_topic, status, error_desc)


class InsertIndexDeltaRequest(ElasticSearchDocument):
    default_topic = 'gl_files'

    def __init__(self, request_id, es_doc):
        super().__init__(request_id, InsertIndexDeltaRequest.default_topic, es_doc=es_doc)
