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
from app.common.enums.job_enum import JobType
from app.copy_catalog.schemas.copy_schemas import DeleteExcessCopiesRequest
from app.copy_catalog.service.copy_delete_workflow import CopyDeleteParam, request_delete_copy_by_id
from app.copy_catalog.service.curd.copy_query_service import query_all_copy_ids_by_resource_id

log = logger.get_logger(__name__)


def delete_excess_copy(resource_id: str, request: DeleteExcessCopiesRequest):
    copy_ids = query_all_copy_ids_by_resource_id(resource_id=resource_id, generated_by=request.generated_by)
    max_count = request.retention_quantity
    if max_count <= 0:
        log.info(f"Policy retention quantity: {max_count} is illegal.")
        return

    current_copy_count = len(copy_ids)
    if current_copy_count <= max_count:
        log.info(f"Current copy count: {current_copy_count} is less than max count: {max_count}.")
        return
    need_delete_count = current_copy_count - max_count
    log.info(f"Need delete {need_delete_count} copies.")
    for copy_id in copy_ids[0:need_delete_count]:
        log.info(f"Delete copy: {copy_id[0]}")
        copy_delete_param = CopyDeleteParam(user_id=request.user_id, strict=True, create_job=True,
                                            job_type=JobType.COPY_EXPIRE.value, is_forced=False)
        # 查询结果为元组，id为元组第一个元素
        request_delete_copy_by_id(copy_id[0], copy_delete_param)