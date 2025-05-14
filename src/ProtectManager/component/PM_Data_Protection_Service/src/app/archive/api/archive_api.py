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
from fastapi import Header, Body, Query

from app.archive.schemas.archive_request import ArchiveRequest
from app.archive.service.service import manual_archive, prepare_archive, is_allow_create_archive_job
from app.common import logger
from app.common.auth.check_ath import CheckAuthModel
from app.common.concurrency import async_route, async_depend
from app.common.enums.rbac_enum import ResourceSetTypeEnum, OperationTypeEnum, AuthOperationEnum
from app.common.exter_attack import exter_attack
from app.common.log.operation_log import operation_log
from app.common.security.jwt_utils import get_user_info_from_token
from app.common.security.right_control import right_control
from app.common.security.role_dict import RoleEnum
from app.protection.object.common.constants import CommonOperationID

api = async_route()
log = logger.get_logger(__name__)


def _resolve_user_info(token: str = Header(..., alias="X-Auth-Token", title="X-Auth-Token",
                                           description="访问令牌")) -> dict:
    return get_user_info_from_token(token)


@exter_attack
@api.post("/archive/manual", status_code=200, summary="手动归档")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    resources="copy:copy_id",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.COPY, operation=OperationTypeEnum.MODIFY,
                              auth_operation_list={AuthOperationEnum.ARCHIVE}, target="archive_request.copy_id")
)
@operation_log(
    name=CommonOperationID.MANUAL_ARCHIVE,
    target="@Archive",
    detail=('archive_request.copy_id!copy.resource_sub_type', 'archive_request.copy_id!copy.resource_name',
            'archive_request.copy_id!copy.display_timestamp!timestamp')
)
def archive(
        user_info: dict = async_depend(_resolve_user_info),
        archive_request: ArchiveRequest = Body(..., description="手动归档请求")
):
    user_id = user_info.get("user-id")
    log.info(f"Manual archive, copy id={archive_request.copy_id}, user id={user_id}")
    manual_archive(archive_request)


@exter_attack
@api.get("/internal/archive/is-allow-create-archive-job", status_code=200, summary="是否允许创建归档任务")
def is_allow_create_archive_job_api(
        resource_id: str = Query(None, description="资源id"),
        storage_id: str = Query(None, description="存储id"),
        copy_id: str = Query(None, description="副本id"),
        is_query_log_copy: bool = Query(False, description="是否支持日志副本归档")
):
    return is_allow_create_archive_job(resource_id, storage_id, copy_id, is_query_log_copy)


@exter_attack
@api.post("/internal/archive/job-examine", status_code=200, summary="归档前置检查")
def examine_archive(
        params: dict = Body(..., description="归档前置检查")
):
    return prepare_archive(params)
