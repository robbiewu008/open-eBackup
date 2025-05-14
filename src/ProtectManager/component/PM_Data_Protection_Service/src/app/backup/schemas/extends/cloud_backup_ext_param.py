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
from pydantic import Field, StrictInt, validator, root_validator
from app.backup.client.archive_client import ArchiveClient
from app.backup.common.constant import CloudBackupConstant
from app.backup.schemas.base_ext_param import BaseExtendParam, BackupExtendParam
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import PolicyTypeEnum, PolicyActionEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException


class CloudBackupExtendParam(BaseExtendParam, BackupExtendParam):
    """云备份"""

    storage_id: str = Field(description="需要备份的备份存储ID")
    open_aggregation: bool = Field(False, description="是否开启聚合功能")
    network_acceleration: bool = Field(False, description="是否开启网络加速")
    is_synthetic_full_copy_period: bool = Field(False, description="是否开启合成全量副本周期")
    synthetic_full_copy_period: StrictInt = Field(None, description="合成全量副本周期")
    auto_index: bool = Field(False, description="是否开启自动索引")

    @staticmethod
    def is_support(application, policy_type, action) -> bool:
        return policy_type is PolicyTypeEnum.backup \
               and application is ResourceSubTypeEnum.CloudBackupFileSystem \
               and action != PolicyActionEnum.snapshot.value

    @validator("storage_id")
    def validate_storage_id(cls, storage_id):
        """
        校验备份存储id是否正确，并存在
        :param storage_id: 备份存储id
        :return: 备份存储id
        """
        if not storage_id:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                        ["backup storage id is empty."])
        storage_obj = ArchiveClient().query_storage_info(storage_id)
        if not storage_obj:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["storage_id"])
        return storage_id

    @root_validator
    def check_acceleration_qos(cls, values):
        """
        1. 校验网络加速是否与限速 2.校验全量副本最大次数
        :param qos_id: 限速策略ID network_acceleration: 是否开启网络加速
        :return: 备份存储id
        """
        if values.get('qos_id') and values.get('network_acceleration'):
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                        ["network_acceleration and qos is mutually exclusive"])

        if values.get("is_synthetic_full_copy_period"):
            # 最大100次增量备份后一次全量
            if (CloudBackupConstant.MIN_SYNTHETIC_FULL_COPY_PERIOD > values.get("synthetic_full_copy_period")) or (
                    values.get("synthetic_full_copy_period") > CloudBackupConstant.MAX_SYNTHETIC_FULL_COPY_PERIOD):
                raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                            ["synthetic_full_copy_period Exceeded the maximum number of times"])
        return values


class CloudBackupSnapshotExtendParam(BaseExtendParam, BackupExtendParam):
    """CloudBackup 防勒索快照备份扩展参数校验"""

    @staticmethod
    def is_support(application, policy_type, action) -> bool:
        return policy_type is PolicyTypeEnum.backup \
               and application is ResourceSubTypeEnum.CloudBackupFileSystem \
               and action == PolicyActionEnum.snapshot.value

    is_security_snap: bool = Field(default=False, description="未勒索副本数据锁定")
