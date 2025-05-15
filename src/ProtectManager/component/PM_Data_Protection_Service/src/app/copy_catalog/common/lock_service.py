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
from app.common import logger
from app.common.context.context import Context
from app.common.event_messages.Rlm.rlm import UnlockRequest, LockRequest
from app.common.events import producer
from app.common.toolkit import modify_job_lock_id
from app.copy_catalog.common import copy_topics
from app.copy_catalog.service.curd.copy_query_service import query_copy_count_by_resource_id, \
    query_replicated_copy_by_resource_id

log = logger.get_logger(__name__)


class LockService:

    @staticmethod
    def unlock_resources(request_id, lock_id):
        # Send message to RLM
        msg = UnlockRequest(
            request_id=request_id, lock_id=lock_id,
        )
        producer.produce(msg)

    @staticmethod
    def copy_delete_lock_resources(request_id, resource_id, copy_id, job_id):
        # add job data lock_id: request_id
        context = Context(request_id)
        context.set("job_id", job_id)
        context.set('lock_id', job_id)
        modify_job_lock_id(context)
        copy_count = query_copy_count_by_resource_id(resource_id)
        lock_resources = [{"id": copy_id, "lock_type": "w"}]
        # 副本删除时，如果只有一个副本并且是复制副本时，配合dme添加FS_DELETE_resource_id锁
        if copy_count == 1:
            replicated_copy = query_replicated_copy_by_resource_id(resource_id)
            if replicated_copy and replicated_copy.get("uuid") == copy_id:
                log.info(f"Last copy is replicated copy, copyId: {copy_id}, request_id:{request_id}")
                lock_resources.append({"id": "FS_DELETE_" + resource_id, "lock_type": "w"})
        # Send message to RLM
        msg = LockRequest(
            request_id=request_id,
            resources=lock_resources,
            wait_timeout=30,  # 设置超时时间30s
            priority=2,  # 优先级
            lock_id=job_id,
            response_topic=copy_topics.CopyTopic.COPY_DELETE_LOCK
        )
        producer.produce(msg)
