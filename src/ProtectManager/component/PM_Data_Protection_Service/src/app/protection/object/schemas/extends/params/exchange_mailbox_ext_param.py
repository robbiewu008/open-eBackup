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
from app.protection.object.schemas.extends.base_ext_param import BaseExtParam


class ExchangeMailboxExtParam(BaseExtParam):
    """
    Exchange邮箱保护高级参数
    """
    backup_continue: Optional[bool] = Field(None, description="邮件/文件夹备份失败继续保护")

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.EXCHANGE_MAILBOX]
