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
from typing import List, Optional, Union
from pydantic import Field, StrictInt, validator
from pydantic.main import BaseModel
from app.backup.client.archive_client import ArchiveClient
from app.backup.schemas.base_ext_param import BaseExtendParam
from app.common.enums.sla_enum import ArchiveScope, ArchiveTypeEnum, CopyTypeEnum, PolicyTypeEnum, RetentionTimeUnit, \
    TimeRangeMonthEnum, TimeRangeWeekEnum, TimeRangeYearEnum, RepositoryProtocolEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException


class SpecifiedScope(BaseModel):
    copy_type: CopyTypeEnum = Field(
        description="指定副本归档的副本类型 年类型year月类型month周类型week")
    generate_time_range: Union[TimeRangeYearEnum, TimeRangeMonthEnum, TimeRangeWeekEnum] = Field(
        description="指定副本产生时间范围单选字符串 按年1-12 按月last/first 按周mon-sun")
    retention_unit: RetentionTimeUnit = Field(None, description="指定副本保留的时间类型 d w MO y")
    retention_duration: int = Field(None, description="保留周期 天 周 年 月")


class ArchiveExtendParam(BaseExtendParam):
    """
    归档扩展参数
    """

    @staticmethod
    def is_support(application, policy_type, action) -> bool:
        return policy_type is PolicyTypeEnum.archiving

    storage_id: str = Field(description="归档存储id")
    archive_target_type: ArchiveTypeEnum = Field(
        description="选择需要归档的副本 1归档所有副本 2指定副本归档")
    archiving_scope: Optional[ArchiveScope] = Field(
        None, description="归档所有副本-归档范围")
    specified_scope: Optional[List[SpecifiedScope]
                              ] = Field(None, description="指定日期副本-归档范围")
    qos_id: Optional[str] = Field(None, description="限速策略id")
    network_access: Optional[bool] = Field(None, description="网络连接")
    auto_retry: bool = Field(description="自动重试")
    auto_retry_times: StrictInt = Field(None, description="自动重试次数,默认3次")
    auto_retry_wait_minutes: StrictInt = Field(
        None, description="自动重试等待时间(分钟),默认5分钟")
    # 只针对导入副本归档
    delete_import_copy: Optional[bool] = Field(None, description="归档后删除原导入副本")
    # 归档类型：cloud/tape
    protocol: RepositoryProtocolEnum = Field(description="归档存储类型")
    # 归档
    auto_index: bool = Field(False, description="是否开启自动索引")

    @validator("storage_id", pre=True)
    def validate_storage_id(cls, storage_id):
        storage_obj = ArchiveClient().query_storage_info(storage_id)
        if not storage_obj:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                        ["storage_id"])
        return storage_id

    @validator('auto_retry_times')
    def check_auto_retry_times_true(cls, auto_retry_times):
        if auto_retry_times is None:
            return auto_retry_times
        if auto_retry_times < 1 or auto_retry_times > 3:
            raise IllegalParamException(
                CommonErrorCodes.ILLEGAL_PARAMS, ["auto_retry_times"])
        return auto_retry_times

    @validator('auto_retry_wait_minutes')
    def check_auto_retry_wait_minutes_true(cls, auto_retry_wait_minutes):
        if auto_retry_wait_minutes is None:
            return auto_retry_wait_minutes
        if auto_retry_wait_minutes < 1 or auto_retry_wait_minutes > 30:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, [
                                        "auto_retry_wait_minutes"])
        return auto_retry_wait_minutes
