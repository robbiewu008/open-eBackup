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

from app.common.enums.schedule_enum import ExecuteType
from fastapi import Body, APIRouter

from app.backup.common.backup_workflow_constant import BackupWorkflowConstants
from app.backup.service import backup_service
from app.common import logger
from app.common.exter_attack import exter_attack
from app.common.schemas.job_schemas import JobSchema

backup_api = APIRouter()
logger = logger.get_logger(__name__)

TAG = "Backup"


@exter_attack
@backup_api.put("/internal/workflows/backup/result", status_code=200, summary="更新备份流程结果,用于备份任务取消时的回调", tags=[TAG])
def update_backup_result(job: JobSchema = Body(None, description="Job Info")):
    logger.info(
        f"backup workflow canceled, job_id:{job.job_id}")
    resource_id = job.source_id
    if not resource_id or not job.data:
        return
    callback_data = json.loads(job.data)
    execute_type = callback_data.get(BackupWorkflowConstants.JOB_CANCEL_CALLBACK_EXECUTE_TYPE)
    # 手动备份不影响遵从度，自动备份时才更新遵从度
    if execute_type and execute_type == ExecuteType.AUTOMATIC.value:
        backup_service.workflow_cancel_callback(resource_id)
