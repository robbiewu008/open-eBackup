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
import datetime
import json
import os
import threading
import time
from concurrent.futures.thread import ThreadPoolExecutor
from contextlib import suppress

from confluent_kafka import Consumer as KafkaConsumer
from confluent_kafka import KafkaException

from app.common.event_messages.topics import TOPICS
from app.common import logger
from app.conf.sensitive_rule_config import config
from app.resource.kafka import topics
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException

LOG = logger.get_logger(__name__)

system_topics = ['AbortRequest', 'ProgressUpdate']


class EsEvent:
    '''
    Emei event with header and message
    '''

    def __init__(
            self, request_id, msg_id=None, default_publish_topic=None, response_topic=None, **message
    ):
        self.msg_id = msg_id
        self.request_id = request_id
        self.default_publish_topic = default_publish_topic
        self.response_topic = response_topic
        self.message = message

    def __repr__(self):
        return f'msg_id={self.msg_id}, request_id={self.request_id}, \
            default_publish_topic={self.default_publish_topic},  response_topic={self.response_topic}'


POOL = ThreadPoolExecutor(10, thread_name_prefix="consumer")


class Consumer:
    '''
    Kafka consumer event messages from subscribed topics.

    1. store last offsets. When recreate new consumer and subscribe again, we should seek
       to last stored offsets
    '''

    def __init__(self, group_id, stop_loop_event=threading.Event(), conf=None):
        self.group_id = group_id
        self.config = config.get_default_consumer_config(
            group_id
        )  # .update(conf or {})
        self.poll_wait_in_seconds = 1.0
        self._kafka_consumer = None
        LOG.info(f'consumer group_id={group_id}, config={self.config}')

        self.stop_loop_event = stop_loop_event or threading.Event()
        self.stop_loop_event.set()

    def __enter__(self):
        return self

    def stop(self):
        LOG.info('stopping consumer')
        self.stop_loop_event.set()

    def is_running(self):
        return not self.stop_loop_event.is_set()

    def kafka_consumer(self):
        ''' Get Kafka consumer '''
        if self._kafka_consumer is None:
            self._kafka_consumer = KafkaConsumer(self.config)
        return self._kafka_consumer

    def subscribe(self, events):
        '''
        Subscribe kafka topics and bind event handler for each topic
        :param: events (Dict[str, handler]) map of topic  name to event handler
        '''
        LOG.info(f'events={events.keys()}')
        self.events = events
        topics = [x for x in events.keys()]
        self.kafka_consumer().subscribe(topics)

    def consume(self):
        """ Try poll new message from subscribed topics """
        kafka_message = self.kafka_consumer().poll(self.poll_wait_in_seconds)

        if kafka_message is None:
            return None

        if kafka_message.error():
            LOG.error(
                f'Consume failed: {kafka_message.topic()}[{kafka_message.partition()}] '
                f'at offset {kafka_message.offset()}, error={kafka_message.error()}'
            )
            return None

        LOG.info(
            f'Consumed Topic({kafka_message.topic()}, {kafka_message.partition()}, {kafka_message.offset()})'
        )
        return kafka_message

    def do_commit(self, kafka_message):
        '''
        Retry to commit message. Exception will raised when failed to commit after n retries
        '''
        retries = 3
        while True:
            try:
                offsets = self.kafka_consumer().commit(
                    message=kafka_message, asynchronous=False
                )
                LOG.debug(f'Commit done offsets={offsets}')
                return offsets
            except KafkaException as ex:
                LOG.exception(f'Commit failed: (retries={retries}), Exception={ex}')
                if retries <= 0:
                    raise KafkaException
                retries -= 1
                time.sleep(1)

    def unpack_event(self, kafka_message):
        """ parse event from kafka message value """
        try:
            s = kafka_message.value().decode()
            topic_info = TOPICS.get(kafka_message.topic())
            if topic_info:
                topic = self.parse_message(topic_info, s)
            else:
                topic = json.loads(kafka_message.value())
                LOG.warn(f'message not verified')
            event = EsEvent(**topic)
            return event
        except ValueError:
            LOG.exception(
                f'Fail parse. Topic({kafka_message.topic()}, {kafka_message.partition()},{kafka_message.offset()}'
            )

    def parse_message(self, topic_info, str_json):
        LOG.debug(f'topic_info={topic_info}')
        event = topic_info.event_cls.parse_raw(str_json)
        d = {field: getattr(event, field) for field in event.fields.keys()}
        return d

    def dispatch(self, topic, event):
        """
        Handle event with register handler.
        :param: key (str) event name
        :param: event (EsEvent) unpacked event
        :return: On failure return the error. Return None when success.
        """
        start_time = datetime.datetime.now()
        try:
            self.events[topic](request=event, **event.message)
        except TypeError as ex:
            LOG.exception(f'params type mismatch for event {topic}')
            return ex
        except Exception as ex:
            LOG.exception(f'unexpect exception for event {topic}')
            return ex
        finally:
            end_time = datetime.datetime.now()
            time_cost = end_time - start_time
            LOG.info(f"topic:{topic},event:{event},start_time:{start_time},end_time:{end_time},time cost:{time_cost}")

    def run(self):
        '''
        Consumer event loop
        :param: event (threading.Event) stop loop event
        '''
        threading.current_thread().name = f'consumer_{self.group_id}'

        if self.is_running():
            raise EmeiStorBizException(CommonErrorCodes.EXIST_SAME_TYPE_JOB_IN_RUNNING,
                                       message='consumer already running')
        self.stop_loop_event.clear()

        LOG.info('Consumer events loop started')
        while not self.stop_loop_event.is_set():
            try:
                kafka_message = self.consume()
                if kafka_message is None:
                    continue

                self.do_commit(kafka_message)
                event = self.unpack_event(kafka_message)
                if event:
                    self.submit(kafka_message, event)
            except RuntimeError as ex:
                LOG.exception(f'Invalid operation when kafka closed: {ex}.')
                self._kafka_consumer = None
            except Exception as ex:
                LOG.exception(f'Unexpected error: {ex}.')

        LOG.info('Exited EsEvents Loop')

    def submit(self, message, event):
        topic = message.topic()
        try:
            POOL.submit(lambda: self.dispatch(topic=topic, event=event))
        except:
            LOG.exception(f"submit task for topic {topic} failed. event:{event}")

    def __exit__(self, type, value, traceback):
        ''' Close kafka consumer connection '''
        self.stop()  # stop event loop if running
        if self._kafka_consumer:
            with suppress(RuntimeError):
                LOG.info('Close kafka consumer')
                self._kafka_consumer.close()


def consume_forever(
        group_id,
        events,
        consume_abort=True,
        on_abort=None,
        consumer_stop_event=threading.Event(),
):
    '''
    Start event loop for requested events and optionally listen to abort messages
    :param: group_id (str)  consumer group
    :param: events (Dict[str, Func])  events handlers.  key=event, value=handler
    :param: consume_abort (bool)  True=consume Emei system workflow abort messages
    :param: on_abort (handler) optional callback when abort message received.
    :param: consumer_stop_event (threading.Event) when set the consumer loop stop.
    '''
    LOG.info(f'group_id={group_id}, events={events}, consume_abort={consume_abort}, on_abort={on_abort}')
    if len(events) == 0:
        raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM,
                                   message="ooops. No events requested")

    event_topics = [x for x, y in events.items()]

    LOG.info(f'Create event_topics={event_topics}')

    partitions = int(os.getenv('KAFKA_DEFAULT_TOPIC_PARTITIONS', 3))
    replication_factor = int(os.getenv('KAFKA_DEFAULT_TOPIC_REPLICATION_FACTOR', 1))
    max_retries = int(os.getenv('KAFKA_CREATE_TOPIC_MAX_RETRIES', 24 * 60))
    topics.create_topics(
        system_topics + event_topics,
        partitions=partitions,
        replication_factor=replication_factor,
        max_retries=max_retries,
    )

    if consume_abort:
        from . import abort

        LOG.info('Consuming abort')
        abort.listen_abort_requests(on_abort)

    LOG.info(f'Starting consumer loop group_id={group_id}')
    with Consumer(group_id, stop_loop_event=consumer_stop_event) as c:
        c.subscribe(events)
        c.run()  # event loop

    if consume_abort:
        abort.stop_listen_abort_requests()

    # exiting consumer loop will also stop the internal abort consumer, since both
    # consumer loops use the same consumer_stop_event
