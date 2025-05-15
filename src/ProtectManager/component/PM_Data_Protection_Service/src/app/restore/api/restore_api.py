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
from fastapi import Body, Header

from app.common.auth.check_ath import CheckAuthModel
from app.common.clients.client_util import ProtectionServiceHttpsClient, is_response_status_ok
from app.common.enums.rbac_enum import ResourceSetTypeEnum, OperationTypeEnum, AuthOperationEnum
from app.common.exter_attack import exter_attack
from app.copy_catalog.util.copy_auth_verify_util import check_copy_operation_auth
from app.copy_catalog.util.copy_util import check_copy_operation_with_restore_limit
from app.restore.schema.restore import RestoreRequestSchema, DownloadRequestSchema, DownloadResponseSchema, \
    check_paths, RestoreRequestStringSchema, RestoreLocation
from app.restore.service import service
from app.common.concurrency import async_route
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.log.operation_log import operation_log
from app.common.schemas.job_schemas import JobSchema
from app.common.security.jwt_utils import get_user_id, get_user_domain_id
from app.common.security.right_control import right_control
from app.common.security.role_dict import RoleEnum
from app.common import logger
from app.restore.service.download import Download

log = logger.get_logger(__name__)

api = async_route()
API_TAG = ["Restore_Manager"]


@exter_attack
@api.post(
    "/restores",
    status_code=200,
    tags=API_TAG,
    description="Restore API",
    summary="创建恢复",
)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    resources=("resource:restore_req.target.env_id", "copy:restore_req.copy_id")
)
@operation_log(
    name=0x206403350005,
    target="@restore",
    detail=('restore_req.copy_id!copy.resource_name',
            'restore_req.copy_id!copy.display_timestamp!timestamp',
            '#result')
)
def create_restore(restore_req: RestoreRequestSchema = Body(..., description="恢复请求体"),
                   token: str = Header(..., alias="X-Auth-Token", description="授权token")):
    user_id = get_user_id(token)
    check_copy_operation_auth(restore_req.copy_id, get_restore_auth_operation(restore_req), get_user_domain_id(token))
    check_copy_operation_with_restore_limit(restore_req)
    return service.submit_restore_job(restore_req, user_id)


def get_restore_auth_operation(restore_req: RestoreRequestSchema):
    restore_location = restore_req.restore_location.value
    if RestoreLocation.origin.value == restore_location:
        return [AuthOperationEnum.ORIGINAL_RESTORE]
    return [AuthOperationEnum.NEW_RESTORE]


@exter_attack
@api.post(
    "/internal/restores",
    status_code=200,
    tags=API_TAG,
    description="Restore API",
    summary="内部接口创建恢复",
)
def create_restore_internal(restore_req_string_schema: RestoreRequestStringSchema = Body(..., description="恢复请求体")):
    restore_req = RestoreRequestSchema.parse_obj(json.loads(restore_req_string_schema.restore_req_string))
    return service.submit_restore_job(restore_req, restore_req_string_schema.user_id)


@exter_attack
@api.post(
    "/internal/restores/action/download",
    status_code=200,
    tags=API_TAG,
    description="Download API",
    summary="下载副本中的指定文件"
)
def download(download_req: DownloadRequestSchema = Body(None, description="下载请求体")):
    check_paths(download_req)
    Download(download_req).download()


@exter_attack
@api.put(
    "/internal/restore/action/abort",
    status_code=200,
    tags=API_TAG,
    description="Restore Abort API",
    summary="恢复终止回调",
)
def abort_job(job: JobSchema = Body(None, description="Job Info")):
    log.info(f"job={job}")
    if job.copy_id is not None:
        response = ProtectionServiceHttpsClient().request(
            "PUT", f"/v1/internal/copies/{job.copy_id}/status",
            body=json.dumps({"status": "Normal"}))
        if not is_response_status_ok(response):
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR)