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
from enum import Enum

from pydantic import BaseModel, Field


class BaseModelConf(BaseModel):
    class Config:
        # allow use field name as parameters of object and alias name
        allow_population_by_field_name = True


class RequestResult(BaseModelConf):
    error_code: int = Field(..., description="error code", alias='ErrorCode')
    error_desc: str = Field(..., description="error desc", alias='ErrorDesc')


class AccountType(str, Enum):
    WINDOWS_USER = 0
    AD_USER = 2
    AD_GROUP = 3


class ResUserInGroup(BaseModelConf):
    account_type: AccountType = Field(..., description="Account type", alias='ACCOUNTTYPE')
    name: str = Field(..., description="Name of the Windows user, AD domain user/user group", alias='NAME')
    RID: int = Field(None, description="User RID.", alias='RID')
    vstore_id: str = Field(None, description="vStore ID", alias='vstoreId')
    vstore_name: str = Field(None, description="vStore name", alias='vstoreName')
