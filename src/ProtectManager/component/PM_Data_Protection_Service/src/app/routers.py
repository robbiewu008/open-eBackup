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
import threading

from app.archive.api.sys_api import sys_api as archive_sys_api
from app.backup.routers.api_router import api_router as protection_api
from app.copy_catalog.api.rest_api import api as copy_api
from app.copy_catalog.api.anti_api import api as anti_api
from app.copy_catalog.kafka import client as copy_client
from app.copy_catalog.service.copy_expire_service import copy_expire_thread
from app.protection.object.router.api_router import api as projected_object_api
from app.replication.api.sys_api import sys_api as replication_sys_api
from app.replication.kafka import replication_workflow
from app.resource.api import api as resource_api
from app.resource.kafka.consumer import resource_client
from app.resource.service.vmware.service_instance_manager import reconnection_vcenter
from app.resource_lock.api.resource_lock_api import lock_api
from app.resource_lock.kafka import messaging_utils
from app.resource_lock.service import async_lock_decorator
from app.restore.routers.api_router import api_router as restore_api
from app.common import logger
from app.common.concurrency import async_fast_api, DEFAULT_ASYNC_POOL
from app.common.exception.exception_handler import setup_exception_handlers
from app.ai_sorter import ai_sorter_router as ai_sorter_api
from app.smart_balance.api.rest_api import smart_balance_router as smart_balance_api

log = logger.get_logger(__name__)

api = async_fast_api()

INTERVAL = 300
DELAY_THIRTY_SECONDS = 30


@api.on_event("startup")
def start():
    resource_start()
    copy_start()
    kafka_client_start()
    replication_workflow.start_log()
    messaging_utils.recover_unsent_messages(async_lock_decorator)


def kafka_client_start():
    copy_client.start()
    resource_client.start()


def copy_start():
    copy_expire_thread.start_copy_expire_thread()


def resource_start():
    # pod重新拉起时连通一次vcenter
    DEFAULT_ASYNC_POOL.submit(reconnection_vcenter)


@api.get("/v1/readiness")
def readiness():
    pass


@api.get("/v1/internal/health")
def readiness():
    # 存活探针健康检查
    pass


setup_exception_handlers(api)
api.include_router(protection_api)
api.include_router(restore_api)
api.include_router(replication_sys_api)
api.include_router(archive_sys_api)
api.include_router(copy_api)
api.include_router(anti_api)
api.include_router(resource_api)
api.include_router(projected_object_api)
api.include_router(lock_api)
api.include_router(ai_sorter_api)
api.include_router(smart_balance_api)
