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
from app.archive.service import service as archive_service
from app.backup.common.constant import ProtectionConstant
from app.backup.service import backup_workflow as backup_workflow_service
from app.common import logger
from app.common.event_messages.Flows.backup import BackupScheduleRequest, BackupInit, BackupDone
from app.common.events.consumer import EsEvent
from app.kafka import client, rlm_client
from app.resource_lock.service import async_lock_decorator
from app.restore.service import service as restore_service

log = logger.get_logger(__name__)

client.register_topic_handler({
    BackupScheduleRequest.default_topic: backup_workflow_service.consume_backup_start,
    BackupScheduleRequest.group_backup_topic: backup_workflow_service.group_backup_start,
    BackupInit.default_topic: backup_workflow_service.backup_context_initialize,
    ProtectionConstant.TOPIC_BACKUP_LOCKED: backup_workflow_service.resource_locked,
    BackupDone.default_topic: backup_workflow_service.backup_done,
    ProtectionConstant.TOPIC_BACKUP_TIMEOUT_CHECK: backup_workflow_service.backup_timeout_check,
    'archive': archive_service.add_resource_lock,
    'schedule.archiving': archive_service.interval_archive,
    'Archive_LockResponse': archive_service.start_archive,
    'archive.copies.detection.complete': archive_service.anti_ransomware_check_result
})

# 资源锁消息消费者单独分组
rlm_client.register_topic_handler({
    'LockRequest': async_lock_decorator.async_lock,
    'UnlockRequest': async_lock_decorator.async_unlock,
    'HasLockRequest': async_lock_decorator.has_lock
})


@client.topic_handler(
    topic="restore",
    prepare=restore_service.restore_prepare,
    lock='lock_resources',
    job_log=("job_log_copy_recovery_schedule_label", "job_status_{payload.job_status|context.job_status|status}_label"),
    failure="protection.restore.done",
    lock_timeout=0
)
def restore(request: EsEvent, **_):
    return restore_service.restore_process(request)


@client.topic_handler(
    topic="protection.restore.done",
    unlock=True,
    terminate=True,
    job_log=("job_log_copy_recovery_complete_label", "job_status_{payload.job_status|context.job_status|status}_label"),
    status="context.job_status",
)
def restore_complete(request: EsEvent, **_):
    log.info(f"task complete. request id:{request.request_id}")


def consumer_start():
    log.info('Started consumer loop on thread')
    client.start()
    rlm_client.start()
