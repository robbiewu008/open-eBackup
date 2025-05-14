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
from typing import List, Optional

from pydantic import Field, root_validator

from app.protection.object.schemas.extends.base_ext_param import BaseExtParam
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.protection.object.common.protection_enums import SmallFileAggregation
from app.common.exception.unified_exception import IllegalParamException
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common import logger

log = logger.get_logger(__name__)
SMALL_FILE_AGGREGATION_SIZE = [128, 256, 1024, 2048, 4096]


class NasShareExtParam(BaseExtParam):

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.NasShare]

    small_file_aggregation: Optional[SmallFileAggregation] = Field(..., description="是否开启小文件聚合")
    aggregation_file_size: Optional[int] = Field(default=0, description="聚合文件大小")
    channels: Optional[int] = Field(1, description="通道数", ge=1, le=40)
    aggregation_file_max_size: Optional[int] = Field(default=0, description="待聚合文件最大大小")
    smb_acl_protection: Optional[bool] = Field(True, description="SMB硬链接开关")
    smb_hardlink_protection: Optional[bool] = Field(True, description="SMB ACL")
    backup_hot_data: Optional[int] = Field(0, description="BACKUP HOT DATA")
    unit: Optional[int] = Field(60, description="UNIT")
    backup_cold_data: Optional[int] = Field(0, description="BACKUP COLD DATA")
    coldUnit: Optional[int] = Field(60, description="COLD UNIT")
    archive_res_auto_index: Optional[bool] = Field(description="对象归档是否启用自动索引")
    tape_archive_auto_index: Optional[bool] = Field(description="磁带归档是否启用自动索引")
    backup_res_auto_index: Optional[bool] = Field(description="备份是否启用自动索引")
    sparse_file_detection: Optional[bool] = Field(False, description="稀疏文件检测")

    @root_validator
    def check_ext_params(cls, values):
        # 1. 聚合文件大小范围为[128, 256, 1024, 2048, 4096] 待聚合文件最大大小范围为[128, 256, 1024, 2048, 4096]
        # 2. 聚合文件大小不能小于待聚合文件最大大小
        if values.get("small_file_aggregation") == SmallFileAggregation.AGGREGATE:
            aggregation_file_size = values.get("aggregation_file_size")
            aggregation_file_max_size = values.get("aggregation_file_max_size")
            if aggregation_file_size not in SMALL_FILE_AGGREGATION_SIZE or aggregation_file_max_size not in \
                    SMALL_FILE_AGGREGATION_SIZE:
                log.error("aggregation param is illegal")
                raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["aggregation param is illegal"])
            if aggregation_file_size < aggregation_file_max_size:
                log.error("aggregation_file_size_tmp is smaller than aggregation_file_max_size")
                raise IllegalParamException(CommonErrorCodes.ERR_PARAM,
                                            ["aggregation_file_size_tmp is smaller than aggregation_file_max_size"])
        return values
