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
from typing import Optional, List

from pydantic import Field, validator

from app.common import logger
from app.common.clients.system_base_client import SystemBaseClient
from app.common.deploy_type import DeployType
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import FileSystemShareType
from app.common.exception.protection_error_codes import ProtectionErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.protection.object.schemas.extends.base_ext_param import BaseExtParam

log = logger.get_logger(__name__)


class CloudBackupExtParam(BaseExtParam):
    """CloudBackup 保护扩展参数校验"""

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.CloudBackupFileSystem]

    share_type: Optional[FileSystemShareType] = Field(description="文件系统共享类型")
    file_system_ids: List[str] = Field(description="文件系统ID")
    if DeployType().is_cyber_engine_deploy_type():
        is_open: bool = Field(default=False, description="自学习开关")
        type: int = Field(default=0, description="自学习类型，0: 按天数 1: 按次数", ge=0, le=1)
        duration: int = Field(default=15, description="自学习周期, 15-60", ge=15, le=60)
        progress: int = Field(default=0, description="自学习进度, 0-60", ge=0, le=60)


    @validator("file_system_ids", each_item=True)
    def check_synchronize_replication_secondary(cls, file_system_id):
        # 安全一体机不进行后续校验
        if DeployType().is_cyber_engine_deploy_type():
            return file_system_id

        # 判断文件系统如果是*同步复制从端*：不允许添加保护
        filesystem_info = SystemBaseClient.query_filesystem(file_system_id)
        log.debug(f"filesystem info: {filesystem_info}")
        if not filesystem_info:
            return file_system_id
        if not filesystem_info.get("id", ""):
            return file_system_id

        remote_replication_ids = filesystem_info.get("remoteReplicationIds", [])
        for remote_replication_id in remote_replication_ids:
            remote_replication_info = SystemBaseClient.query_replication_pair(remote_replication_id)
            if not remote_replication_info:
                return file_system_id
            if remote_replication_info.get("id", "") and \
                    remote_replication_info.get("replicationModel", "") == 1 and \
                    ~remote_replication_info.get("primary", False):
                raise EmeiStorBizException(ProtectionErrorCodes.CLOUDBACKUP_SYNCHRONOUS_REPLICATION_SECONDARY,
                                           filesystem_info.get("name", ""))

        # 主存防勒索兼容双活从端添加保护，不下发备份任务
        if DeployType().is_hyper_detect_deploy_type():
            return file_system_id

        hyper_metro_pair_ids = filesystem_info.get("hyperMetroPairIds", [])
        for hyper_metro_pair_id in hyper_metro_pair_ids:
            hyper_metro_pair_info = SystemBaseClient.query_hyper_metro_pair(hyper_metro_pair_id)

            if not hyper_metro_pair_info:
                return file_system_id

            # 双活从端不允许保护
            if not hyper_metro_pair_info.get("primary"):
                raise EmeiStorBizException(ProtectionErrorCodes.CLOUDBACKUP_SYNCHRONOUS_REPLICATION_SECONDARY,
                                           filesystem_info.get("name", ""))
        return file_system_id
