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
import json
import uuid

from pydantic import BaseModel, Field, validator

MAX_TOPIC_NAME_LEN = 64


class EventMessage(BaseModel):
    msg_id: uuid.UUID = None
    request_id: uuid.UUID = Field(..., description='Operation request id')
    default_publish_topic: str = Field(..., description='Default event topic')
    response_topic: str = Field(default=None,
                                description='Optional: instruct consumer handler to produce to response_topic')

    @validator('msg_id', always=True)
    def msg_id_generate_default_unique_id(cls, v):
        return uuid.uuid4()

    class Config:
        fields = {
            'default_publish_topic': {'const': True, 'max_length': MAX_TOPIC_NAME_LEN}
        }


class EventBase:
    '''
    Base Emei Event Message

    :field: msg_id (str)   unique message id
    :field: request_id (str) flow id
    :field: default_publish_topic (str)   the default topic to produce
    '''

    def __init__(self, request_id, default_publish_topic, response_topic=''):
        self.msg_id = str(uuid.uuid4())
        self.request_id = request_id
        self.default_publish_topic = default_publish_topic
        self.response_topic = response_topic


class CommonEvent(EventBase):
    def __init__(self, topic, request_id=None, **kwargs):
        request_id = request_id if request_id else str(uuid.uuid4())
        EventBase.__init__(self, request_id, topic)
        for k, v in kwargs.items():
            setattr(self, k, v)


class EventResponseBase(EventBase):
    '''
    Base Emei Event Message

    :field: msg_id (str)   unique message id
    :field: request_id (str) flow id
    :field: default_publish_topic (str)   the default topic to produce
    '''

    def __init__(self, request_id, default_publish_topic, status='success', error_desc=''):
        super().__init__(request_id, default_publish_topic)
        self.status = status
        self.error_desc = error_desc


class ElasticSearchDocument(EventBase):
    '''
    Insert elasticsearch document message
    '''

    def __init__(self, request_id, default_publish_topic, es_doc):
        super().__init__(request_id, default_publish_topic)
        self.es_doc = es_doc

    @staticmethod
    def on_delivery(msg):
        return json.loads(msg['es_doc'])
