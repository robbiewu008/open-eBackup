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
from sqlalchemy.orm import Session

from app.base.db_base import database
from app.common import logger, toolkit
from app.common.enums.job_enum import JobStatus, JobLogLevel
from app.job.models.job_models import JobTable

logger = logger.get_logger(__name__)


def get_job_by_id_with_session(job_id: str) -> JobTable:
    with database.session() as session:
        return get_job_by_id(session, job_id)


def get_job_by_id(db: Session, job_id: str) -> JobTable:
    return db.query(JobTable).filter(JobTable.job_id == job_id).first()


def record_job_step(job_id: str, request_id: str, job_step_label: str,
                    log_level: JobLogLevel = JobLogLevel.INFO,
                    log_info_param=None, log_detail=None, log_detail_param=None):
    if not log_info_param:
        log_info_param = []
    log_step_req = toolkit.build_update_job_log_request(job_id,
                                                        job_step_label,
                                                        log_level,
                                                        log_info_param,
                                                        log_detail,
                                                        log_detail_param)
    toolkit.modify_task_log(request_id, job_id, log_step_req)


def update_job(job_id: str, request_id: str, status: JobStatus, progress=None):
    update_req = {
        "status": status.value
    }
    if status is JobStatus.RUNNING:
        update_req.update(progress=progress)
    elif status is JobStatus.SUCCESS or status is JobStatus.PARTIAL_SUCCESS:
        update_req.update(progress=100)
    toolkit.complete_job_center_task(request_id, job_id, update_req)
