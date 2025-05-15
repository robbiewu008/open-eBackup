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
from fastapi import Header, Body

from app.archive.schemas.archive_request import ArchiveMsg
from app.archive.service.service import handle_schedule_archiving
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
from app.replication.schemas.replication_request import ReplicationRequest
from app.replication.service.replication_service import reverse_replicate
from app.common.config import settings
from app.resource.service.common.resource_service import query_current_cluster
from app.backup.common.constant import ProtectionConstant

api = async_route()
log = logger.get_logger(__name__)


def _resolve_user_info(token: str = Header(..., alias="X-Auth-Token", title="X-Auth-Token",
                                           description="访问令牌")) -> dict:
    return get_user_info_from_token(token)


@exter_attack
@api.post("/replication", status_code=200, summary="反向复制")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    resources="copy:copy_id",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.COPY, operation=OperationTypeEnum.MODIFY,
                              auth_operation_list={AuthOperationEnum.REPLICATION}, target="replication_request.copy_id")
)
@operation_log(
    name=CommonOperationID.MANUAL_REPLICATION,
    target="@Replication",
    detail=('replication_request.copy_id!copy.resource_sub_type', 'replication_request.copy_id!copy.resource_name',
            'replication_request.copy_id!copy.display_timestamp!timestamp')
)
def replicate(
        user_info: dict = async_depend(_resolve_user_info),
        replication_request: ReplicationRequest = Body(..., description="反向复制请求")
):
    user_id = user_info.get("user-id")
    log.info(f"Reverse replication, copy id={replication_request.copy_id}, user id={user_id}")

    reverse_replicate(user_id, replication_request)


def get_dispatch_archive_log_data(params):
    local_device_esn = settings.get_key_from_config_map(ProtectionConstant.CLUSTER_CONFIG,
                                                        ProtectionConstant.CLUSTER_ESN)
    current_cluster = query_current_cluster(local_device_esn)
    if not current_cluster:
        return local_device_esn
    return current_cluster.cluster_name


@exter_attack
@api.post("/archive/dispatch", status_code=200, summary="归档请求")
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},)
@operation_log(
    name=CommonOperationID.RECEIVE_ARCHIVE_DISPATCH,
    target="@Archive",
    detail=(get_dispatch_archive_log_data)
)
def receive_archive_dispatch(
        msg: ArchiveMsg = Body(..., description="归档请求")):
    """
    接收java端传来的备份完成后的归档消息
    :param msg: 归档消息
    :return:
    """
    log.info(f"Receive archive msg from java. request id:{msg.request_id}")
    handle_schedule_archiving(msg)
