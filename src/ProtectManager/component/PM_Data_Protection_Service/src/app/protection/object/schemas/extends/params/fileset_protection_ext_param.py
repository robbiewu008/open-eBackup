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

from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException
from app.protection.object.schemas.extends.base_ext_param import BaseExtParam
PROTECTION_REGEX = "^(.+)[\.]{1}(sh|bat){1}$"
SMALL_FILE_AGGREGATION_SIZE = [128, 256, 1024, 2048, 4096]


class FilesetProtectionExtParam(BaseExtParam):
    """
    校验主机文件集保护的参数
    """
    consistent_backup: Optional[bool] = Field(False, description="一致性备份")
    cross_file_system: Optional[bool] = Field(False, description="跨文件系统备份")
    backup_nfs: Optional[bool] = Field(False, description="备份NFS")
    backup_smb: Optional[bool] = Field(False, description="备份SMB")
    channels: Optional[int] = Field(1, description="通道数", ge=1, le=40)
    sparse_file_detection: Optional[bool] = Field(False, description="稀疏文件检测")
    backup_continue_with_files_backup_failed: Optional[bool] = Field(False, description="备份时跳过不存在的文件和目录")
    small_file_aggregation: Optional[bool] = Field(False, description="小文件聚合")
    aggregation_file_size: Optional[int] = Field(False, description="聚合文件大小")
    aggregation_file_max_size: Optional[int] = Field(False, description="待聚合文件最大大小")
    pre_script: Optional[str] = Field(None, description="备份前运行脚本", regex=PROTECTION_REGEX, max_length=8192)
    post_script: Optional[str] = Field(None, description="备份成功运行脚本", regex=PROTECTION_REGEX, max_length=8192)
    failed_script: Optional[str] = Field(None, description="备份失败运行脚本", regex=PROTECTION_REGEX, max_length=8192)
    archive_res_auto_index: Optional[bool] = Field(description="对象归档是否启用自动索引")
    tape_archive_auto_index: Optional[bool] = Field(description="磁带归档是否启用自动索引")
    backup_res_auto_index: Optional[bool] = Field(description="备份是否启用自动索引")
    snapshot_size_percent: Optional[int] = Field(None, description="备份容量百分比", ge=0, le=100)

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.Fileset]

    @root_validator
    def check_ext_params(cls, values):
        # 1. 小文件聚合参数选择后
        # 2. 聚合文件大小范围为[128, 256, 1024, 2048, 4096] 待聚合文件最大大小范围为[128, 256, 1024, 2048, 4096]
        # 3. 聚合文件大小不能超过待聚合文件最大大小
        if values.get("small_file_aggregation"):
            aggregation_file_size_tmp = values.get("aggregation_file_size")
            aggregation_file_max_size_tmp = values.get("aggregation_file_max_size")
            if not aggregation_file_size_tmp or not aggregation_file_max_size_tmp:
                raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["aggregation param is illegal"])
            if aggregation_file_size_tmp not in SMALL_FILE_AGGREGATION_SIZE or aggregation_file_max_size_tmp not in \
                    SMALL_FILE_AGGREGATION_SIZE:
                raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["aggregation param is illegal"])
            if aggregation_file_size_tmp < aggregation_file_max_size_tmp:
                raise IllegalParamException(CommonErrorCodes.ERR_PARAM,
                                            ["aggregation_file_size_tmp is smaller than aggregation_file_max_size"])
        return values
