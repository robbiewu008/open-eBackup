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
from app.common.clients import job_center_client
from app.common.event_messages.Sla.sla_message import SlaChangedRequest
from app.common.events.topics import RESOURCE_DELETED_TOPIC
from app.copy_catalog.common.copy_topics import CopyTopic
from app.copy_catalog.service.copy_delete_workflow import (
    request_delete_copy_by_id,
    handle_protection_removed,
    handle_copy_delete_locked,
    process_copy_delete_context_init, CopyDeleteParam
)
from app.copy_catalog.service.copy_expire_service import cache_recent_expiring_copies, handle_copy_check, \
    copy_expire_thread
from app.copy_catalog.service.curd.copy_delete_service import (
    handle_copy_deleted,
    handle_resource_removed,
)
from app.copy_catalog.service.curd.copy_create_service import save_copies
from app.copy_catalog.service.curd.copy_update_service import handle_resource_added, \
    resource_sla_name_of_copy_update_handle, update_copy_protection_sla_name_by_sla_id
from app.settings import client, resource_client
from app.common.event_messages.Resource.resource import ResourceAddedRequest
from app.common.events.consumer import EsEvent
from app.common.logger import get_logger
from app.common.redis_session import redis_session

log = get_logger(__name__)


@client.topic_handler(CopyTopic.COPY_RETENTION_TOPIC)
def copy_retention_handler(request: EsEvent, **ignores):
    request_id = request.request_id
    log.info(f"copy retention request: request_id={request_id}, ignores={ignores}")
    copy_expire_thread.check_copy_expire_thread()
    cache_recent_expiring_copies()


@client.topic_handler(CopyTopic.COPY_CHECK_TOPIC)
def copy_check_handler(request: EsEvent, copy_id: str):
    request_id = request.request_id
    log.info(f"copy check handler: request_id={request_id}")
    handle_copy_check(copy_id)


@client.topic_handler(CopyTopic.COPY_DELETE_REQUEST_TOPIC)
def copy_delete_request_handler(
        request: EsEvent,
        copy_id: str,
        user_id: str,
        job_id: str,
        response_topic: str,
        response_param: str
):
    request_id = request.request_id
    log.info(f"copy delete request handler: request_id={request_id}")
    redis_session.hset(request_id, "response_topic", response_topic)
    redis_session.hset(request_id, "response_param", response_param)
    copy_delete_param = CopyDeleteParam(user_id=user_id, job_id=job_id)
    request_delete_copy_by_id(copy_id, copy_delete_param)


@client.topic_handler(CopyTopic.COPY_DELETED_TOPIC)
def copy_deleted_handler(request: EsEvent):
    log.info(f'Post-processing after deleting a copy,request_id:{request.request_id}')
    handle_copy_deleted(request.request_id)


@client.topic_handler(CopyTopic.COPY_SAVE_TOPIC)
def copy_information_handler(request: EsEvent):
    save_copies(request.request_id)


@client.topic_handler(CopyTopic.PROTECTION_REMOVE_EVENT)
def protection_removed_handler(request: EsEvent, resource_id: str, **__):
    handle_protection_removed(request.request_id, resource_id)


@client.topic_handler(RESOURCE_DELETED_TOPIC)
def resource_removed_handler(request: EsEvent, resource_id: str):
    handle_resource_removed(request.request_id, resource_id)


@client.topic_handler(ResourceAddedRequest.default_topic)
def resource_added_handler(request: EsEvent, resource_id: str):
    # 如果资源删除之后又重新注册上来，资源id不会变，这个时候如果有副本，需要将副本中的资源状态改为存在
    handle_resource_added(request.request_id, resource_id)


@client.topic_handler(CopyTopic.COPY_DELETE_LOCK)
def copy_delete_locked_handler(request: EsEvent, error_desc, status):
    if not job_center_client.query_is_job_present(request.request_id):
        return
    handle_copy_delete_locked(request.request_id, error_desc, status)


@client.topic_handler(CopyTopic.COPY_DELETE_INIT)
def init_copy_delete_context(request: EsEvent, **payload):
    process_copy_delete_context_init(**dict(**payload, request_id=request.request_id))


@resource_client.topic_handler(SlaChangedRequest.default_topic)
def resource_sla_name_of_copy_update_handler(request: EsEvent, **payload):
    # sla名称变更后，修改副本的resource_properties中sla_name值
    resource_sla_name_of_copy_update_handle(request.request_id, **payload)
    update_copy_protection_sla_name_by_sla_id(request.request_id, **payload)
