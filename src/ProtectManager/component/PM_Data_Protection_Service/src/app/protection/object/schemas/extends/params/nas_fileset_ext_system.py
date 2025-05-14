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
from app.protection.object.common.protection_enums import NasProtocolType
from app.protection.object.schemas.extends.base_ext_param import BaseExtParam


class NasFileSystemExtParam(BaseExtParam):
    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.NasFileSystem]

    protocol: int = Field(..., description="选择文件系统创建的协议")
    agents: str = Field(None, description="选择的代理主机")
    archive_res_auto_index: Optional[bool] = Field(description="对象归档是否启用自动索引")
    tape_archive_auto_index: Optional[bool] = Field(description="磁带归档是否启用自动索引")
    backup_res_auto_index: Optional[bool] = Field(description="备份是否启用自动索引")


    @root_validator
    def check_ext_params(cls, values):
        # 选择协议，必选且为cifs或者nfs
        if values.get('protocol') not in [NasProtocolType.CIFS.value, NasProtocolType.NFS.value]:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                        ["param of agents is not need when proxy host mode is auto"])
        return values
