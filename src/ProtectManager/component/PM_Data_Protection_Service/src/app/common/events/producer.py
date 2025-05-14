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
import threading
import uuid
import asyncio
from asyncio import new_event_loop

from confluent_kafka import KafkaException
import confluent_kafka

from app.common import logger
from app.common.events.events import is_subclass_of_event_message
from app.common.config import settings
from app.common.sensitive.sensitive_word_filter_util import sensitive_word_filter
from app.common.constants.constant import SecurityConstants, CallbackStatus

LOG = logger.get_logger(__name__)


class AIOProducer:
    def __init__(self, confluent_producer, loop=None):
        self._loop = loop or asyncio.new_event_loop()
        self._producer = confluent_producer

    def produce(self, topic, value, key, req_id=None):
        """
        An awaitable produce method.
        """
        result = self._loop.create_future()

        def ack(err, msg, event, req_id):
            if err:
                LOG.error(
                    f'Fail to send {msg.topic()}[{msg.partition()}] at offset {msg.offset()}, err={err}, '
                    f'request_id: {req_id}.'
                )
                result.set_result(CallbackStatus.FAIL)
            else:
                LOG.info(
                    f'Successed to send msg {msg.topic()}[{msg.partition()}] at offset ' f'{msg.offset()}, '
                    f'request_id: {req_id}.'
                )
                result.set_result(CallbackStatus.SUCCESS)
            event.set()

        event = threading.Event()
        LOG.info(f'async begin produce message, request_id: {req_id}.')
        self._producer.produce(
            topic, value=value, key=key, callback=lambda err, msg: ack(err, msg, event, req_id)
        )
        self._producer.flush()
        # 等待ack被调用完后再返回result
        event.wait()
        LOG.debug(f'wait success! event is true now, request_id: {req_id}.')

        return result


single_producer = confluent_kafka.Producer({
    "bootstrap.servers": settings.BOOTSTRAP_SERVER,
    "security.protocol": "SASL_SSL",
    "sasl.mechanism": "PLAIN",
    "sasl.username": settings.KAFKA_USERNAME,
    "sasl.password": settings.get_kafka_password(),
    "ssl.ca.location": SecurityConstants.INTERNAL_CA_FILE,
})


def get_event_value(event, on_custom_message=None):
    if on_custom_message:
        d = on_custom_message(event.dict())
        return json.dumps(d).encode('utf-8')
    return event.json()


def produce(event, response_topic=None, on_delivery=None, msg_key=None, trace=False):
    LOG.debug(f'message type: {type(event)}')
    if trace:
        LOG.info(f"event: {event}")
    if msg_key is None:
        msg_key = str(event.msg_id)
    if is_subclass_of_event_message(event):
        produce_event_message(event, response_topic, on_delivery, msg_key)
    else:
        __produce_not_pydantic_event(event, response_topic, on_delivery, msg_key)


def produce_event_message(event, response_topic=None, on_delivery=None, msg_key=None):
    """
    Send EMEI event message to kafka

    :param: event (es_event_messages.EventMessage):  Emei event class
    :param: response_topic (str)  topic name to send explictly to
    :param: on_delivery (callback)  Enable to manipulate the JSON message before sending it to Kafka.

    Example for on_delivery:
    Suppose you need to send JSON document to a topic that is consumed by a
    kafka-connector that fills an ElasticSearch index. So, to strips the Emei message headers, such as to
    request_id and msg_id, we will use the on_delivery callback that will return the desired JSON.

    msg = es_event_messages.ES.InsertDocument(request_id='xxx', es_doc=fs_index_document)
    es_event.produce(msg, on_delivery=lambda event: event['es_doc'])
    """
    LOG.info(f'start to produce message, event_obj:{event}, response_topic={response_topic}',
             extra={'request_id': event.request_id})
    event.msg_id = uuid.uuid4()
    topic = response_topic or event.default_publish_topic
    try:
        value = get_event_value(event, on_custom_message=on_delivery)
        req_id = event.request_id
        loop = new_event_loop()
        aio_producer = AIOProducer(single_producer, loop)
        result = aio_producer.produce(topic=topic, value=value, key=msg_key, req_id=req_id)
        LOG.info(f'produce msg result: {result}, topic:{topic}, request_id: {req_id}.')
        if not result.result() or result.result() == CallbackStatus.FAIL:
            raise Exception(f"produce msg failed, topic:{topic}, request_id: {req_id}")
    except KafkaException as err:
        LOG.exception(
            f'produce msg failed, topic:{topic}, request_id: {req_id}, {err} '
        )
        raise
    except Exception as ex:
        LOG.exception(f'produce msg failed, topic:{topic}, request_id {req_id}, {ex}')
        raise


def __produce_not_pydantic_event(
    event, response_topic=None, on_delivery=None, msg_key=None
):
    event_dict = event.__dict__
    LOG.debug(
        f'event_obj:{event}, event_dict={sensitive_word_filter(event_dict)}, response_topic={response_topic}',
        extra={'request_id': event.request_id}
    )

    # override default response topic
    topic = event_dict['default_publish_topic']
    if response_topic:
        topic = response_topic

    LOG.info(
        f'sending topic={topic}, request_id={event.request_id}',
        extra={'request_id': event.request_id, 'topic': topic, 'event_obj': sensitive_word_filter(event_dict)},
    )
    try:
        if on_delivery:
            event_dict = on_delivery(event_dict)

        value = json.dumps(event_dict).encode('utf-8')
        req_id = event.request_id
        loop = new_event_loop()
        aio_producer = AIOProducer(single_producer, loop)
        result = aio_producer.produce(topic, value=value, key=msg_key, req_id=req_id)
        LOG.info(f'produce msg result: {result}, topic:{topic}, request_id: {req_id}.')
        if not result.result() or result.result() == CallbackStatus.FAIL:
            raise Exception(f"produce msg failed, topic:{topic}, request_id: {req_id}")
    except KafkaException as err:
        LOG.exception(
            f'produce msg failed, topic: {topic}, request_id: {req_id}, {err} '
        )
        raise
    except Exception as ex:
        LOG.exception(f'Fail send message. topic: {topic}, request_id {req_id}, {ex}')
        raise
