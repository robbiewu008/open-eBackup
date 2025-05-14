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

from pydantic import Field, root_validator

from app.common import logger
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException
from app.protection.object.common.protection_enums import NasProtocolType
from app.protection.object.schemas.extends.base_ext_param import BaseExtParam

log = logger.get_logger(__name__)


class NdmpExtParam(BaseExtParam):
    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.NdmpBackupSet]

    protocol: int = Field(..., description="选择文件系统创建的协议")
    agents: str = Field(None, description="选择的代理主机")
    filters: Optional[List[str]] = Field(None, description="过滤文件/目录")
    archive_res_auto_index: Optional[bool] = Field(description="归档是否启用自动索引")
    backup_res_auto_index: Optional[bool] = Field(description="备份是否启用自动索引")

    @staticmethod
    def get_indices(filter_str, substr):
        indices = [
            index
            for index, char in enumerate(filter_str)
            if filter_str[index:index + len(substr)] == substr
        ]
        return indices

    @root_validator
    def check_ext_params(cls, values):
        # 选择协议，必选且为NDMP
        if values.get('protocol') not in [NasProtocolType.NDMP.value]:
            log.error("protocol param is illegal")
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["protocol param is illegal"])
        """
        1.星号（*）是通配符，必须是字符串的第一个或最后一个字符。
        2.每个字符串最多可以有两个星号。
        3.文件名或目录名中的逗号前必须有一个反斜杠。
        4.排除列表最多可以包含32个名称。
        5.匹配字符串最大长度为255个字符。
        """
        if 'filters' not in values:
            return values
        filters = values.get('filters')
        if filters is None or len(filters) > 32:
            log.error("filters param is illegal")
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["filters param is illegal"])
        for filter_str in filters:
            if filter_str is None or len(filter_str) == 0 or len(filter_str) > 255:
                log.error("filters param is illegal")
                raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["filters param is illegal"])
            indices = cls.get_indices(filter_str, "*")
            for index in indices:
                if index not in [0, len(filter_str) - 1]:
                    log.error("filters param is illegal")
                    raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["filters param is illegal"])
            indices = cls.get_indices(filter_str, ",")
            for index in indices:
                if filter_str[index - 1] != '\\':
                    log.error("filters param is illegal")
                    raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["filters param is illegal"])
        return values
