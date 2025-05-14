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

from pydantic import Field

from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.logger import get_logger
from app.protection.object.common.constants import HDFSFilesetExtParam
from app.protection.object.schemas.extends.base_ext_param import BaseExtParam

log = get_logger(__name__)


class HDFSFilesetExtParam(BaseExtParam):
    """
    校验HDFS保护的参数
    """

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.HDFSFileset]

    # 脚本参数不是必传的
    pre_script: Optional[str] = Field(None, description="用于给资源绑定SLA增加前置脚本",
                                      regex=HDFSFilesetExtParam.PROTECTION_REGEX,
                                      max_length=8192)
    post_script: Optional[str] = Field(None, description="用于给资源绑定SLA增加后置脚本",
                                       regex=HDFSFilesetExtParam.PROTECTION_REGEX,
                                       max_length=8192)
    failed_script: Optional[str] = Field(None, description="用于给资源绑定SLA增加失败处理脚本",
                                         regex=HDFSFilesetExtParam.PROTECTION_REGEX,
                                         max_length=8192)
    archive_res_auto_index: Optional[bool] = Field(description="归档是否启用自动索引")
    backup_res_auto_index: Optional[bool] = Field(description="备份是否启用自动索引")
