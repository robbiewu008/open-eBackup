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
import os
import random
import threading
import time
import secrets

from app.common import logger
from app.common.events import config
from app.common.events import consumer
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.redis_session import redis_session

LOG = logger.get_logger(__name__)
ABORT_REQUEST_TOPIC = 'AbortRequest'

aborted_requests = {}  # Map of aborted requests. key=request id, value=timestamp
consumer_thread = None  # Abort consumer event loop

# Optional callbackfor received abort
on_abort_callback = None
abort_listener_stop_event = None


def generate_unique_consumer_group_id():
    ''' Generate consumer unique group id '''
    random.seed(os.getpid())
    num1 = int(round(1000 * time.time()))
    secret_generator = secrets.SystemRandom()
    num2 = secret_generator.randint(0, 10000)
    return f'abort_{num1}_{num2}'


def get_request_id_cached_days(request_id):
    ''' Calculate days cached of requests.  If request not exists return -1 '''
    timestamp = aborted_requests.get(request_id, None)

    if timestamp is None:
        return -1

    days = (datetime.datetime.utcnow().timestamp() - timestamp) / (24 * 60 * 60)
    return days


def fetch_all_aborted_requests():
    global aborted_requests
    aborted_requests = redis_session.get(config.REDIS_ABORT_KEY_ENTRY) or {}
    LOG.info(f'Fetch aborted_requests={aborted_requests}')


def uncached_aborted_requests():
    ''' Uncache request with more than 24h duration '''
    global aborted_requests

    n = len(aborted_requests)
    aborted_requests = {
        x: y for x, y in aborted_requests.items()
        if get_request_id_cached_days(x) < 1
    }
    deleted = n - len(aborted_requests)
    if deleted > 0:
        LOG.info(f'Delete requests {deleted}')


def is_aborted(request_id):
    ''' Check if request id is aborted '''
    return str(request_id) in aborted_requests


def on_abort(request, timestamp):
    ''' Abort event internal handler '''
    request_id = str(request.request_id)
    LOG.info(f'request_id={request_id}, timestamp={timestamp}', extra={'request_id': request_id})

    aborted_requests[request_id] = timestamp
    if on_abort_callback:
        LOG.info('Call user abort callback', extra={'request_id': request_id})
        try:
            on_abort_callback(request, timestamp)
        except Exception as ex:
            LOG.exception(f'Unexpected error on abort callback: {ex}')

    uncached_aborted_requests()


def stop_listen_abort_requests():
    LOG.info('exiting abort event loop')
    global abort_listener_stop_event
    if abort_listener_stop_event:
        abort_listener_stop_event.set()
        abort_listener_stop_event = None


def listen_abort_requests(abort_callback=None):
    '''
    Event loop for broadcasted abort request. The consumer has a unique id, in order to
    be able to receive the broadcasted abort request

    :param: abort_callback (handler) optional callback
    '''
    global on_abort_callback
    global consumer_thread
    global abort_listener_stop_event

    on_abort_callback = abort_callback
    if abort_listener_stop_event is not None:
        raise EmeiStorBizException(CommonErrorCodes.EXIST_SAME_TYPE_JOB_IN_RUNNING,
                                   message="allow running single abort request litener")

    abort_listener_stop_event = threading.Event()

    group_id = generate_unique_consumer_group_id()
    LOG.info(f'Starting abort event loop with cosumer group_id={group_id}, abort_callback={abort_callback}')

    fetch_all_aborted_requests()

    # Attention !  Call consume_forever with consumer_abort=False to avoid recursion
    consumer_thread = threading.Thread(
        target=consumer.consume_forever,
        args=(
            group_id,
            {ABORT_REQUEST_TOPIC: on_abort},
            False,
            None,
            abort_listener_stop_event,
        ),
    )
    consumer_thread.start()
    LOG.info('started')
