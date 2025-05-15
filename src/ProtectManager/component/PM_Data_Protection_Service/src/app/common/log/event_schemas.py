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
from enum import IntEnum
from typing import List, Any

from pydantic import BaseModel, Field


class SendEventReq(BaseModel):
    userId: str = Field(None, description="用户的id")
    eventId: str = Field(description="事件id")
    eventLevel: str = Field(default="INFO", description="事件级别")
    eventParam: List[str] = Field(None, description="事件参数")
    eventSequence: int = Field(None, description="序列号")
    eventTime: int = Field(None, description="事件时间")
    moId: str = Field(default="", description="网元id")
    moIP: str = Field(default="", description="网元ip")
    moName: str = Field(default="", description="网元名称")
    sourceId: str = Field(description="告警来源id")
    sourceType: str = Field(description="事件来源类型")
    isSuccess: bool = Field(None, description="是否成功")
    resource_id: str = Field(None, description="资源id")
    legoErrorCode: str = Field(None, description="lego错误码")
    dmeToken: str = Field(None, description="DME Token")
    hcsToken: str = Field(None, description="HCS Token")


class BatchOperation(BaseModel):
    id: str = Field(description="操作id")
    targetName: str = Field("", description="操作对象名称")
    targetNameParam: List[str] = Field(None, description="操作对象名称参数")
    errorCode: int = Field(0, description="错误码")
    errorMessageKey: str = Field(description="错误信息描述")
    taskName: str = Field("", description="任务名称")
    optDetail: str = Field(None, description="操作详情")
    detailParam: List[str] = Field(None, description="操作详情参数")


class BatchOperationResult(BaseModel):
    totalCount: int = Field(0, description="总体操作数量")
    succeedCount: int = Field(0, description="成功操作数量")
    failCount: int = Field(0, description="失败操作数量")
    results: List[BatchOperation] = Field(None, description="操作结果")


class LogRank(IntEnum):
    INVALID = -1,
    INFO = 0,
    WARNING = 1,
    MINOR = 2,
    MAJOR = 3,
    CRITICAL = 4


class OperationConfig(BaseModel):
    name: str
    target: str
    detail: Any
