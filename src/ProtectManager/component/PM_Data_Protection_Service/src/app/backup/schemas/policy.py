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
import json
from datetime import date, time, datetime
from typing import List, Optional, Union
from uuid import UUID

from pydantic import BaseModel, Field, Json, validator, StrictInt, root_validator

from app.backup.common.validators.business.trigger.customize_trigger_validator import CustomizeTriggerValidator
from app.backup.schemas.base_ext_param import BaseExtendParam
from app.common.enums.sla_enum import TriggerActionEnum, TriggerEnum, PolicyActionEnum, \
    PolicyTypeEnum, RetentionTypeEnum, RetentionTimeUnit, WeekDaysEnum, IntervalUnit
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException


class Retention(BaseModel):
    retention_type: RetentionTypeEnum = Field(description="保留类型：1-永久保留 2-临时保留")
    duration_unit: Optional[RetentionTimeUnit] = Field(
        None, description="保留周期单位[d/w/MO/y]")
    retention_duration: Optional[StrictInt] = Field(None, description="保留周期")

    class Config:
        orm_mode = True


def convert_list_to_str(week_days):
    if isinstance(week_days, list):
        try:
            return ",".join([WeekDaysEnum.get_value(key) for key in week_days])
        except (ValueError, KeyError):
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                        ["days_of_week"])
    return week_days


def convert_str_to_list(week_days):
    if isinstance(week_days, str):
        try:
            return [WeekDaysEnum.get_key(key) for key in week_days.split(",")]
        except (ValueError, KeyError):
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                        ["days_of_week"])
    return week_days


class ScheduleBase(BaseModel):
    trigger: TriggerEnum = Field(
        description="调度触发类型,1-周期执行，2-备份完立即执行，3-备份完指定时间执行, 4-指定时间周期执行")
    interval: StrictInt = Field(None, description="调度间隔周期[分/小时/天/周/月]")
    interval_unit: Optional[IntervalUnit] = Field(
        None, description="调度间隔周期单位[m/h/d]")
    start_time: Optional[Union[datetime, date]] = Field(
        None, description="首次开始时间，格式 [YYYY/MM/DD HH:mm:ss]/[YYYY/MM/DD]")
    window_start: Optional[time] = Field(
        None, description="时间窗开始时间，格式[HH:mm:ss]")
    window_end: Optional[time] = Field(
        None, description="时间窗结束时间，格式 HH:mm:ss")
    days_of_month: Optional[str] = Field(
        None, description="每月的某些天，可多选，格式 1-20,21,23", regex="^\d+(?:-\d+)?(?:,\d+(?:-\d+)?)*$")
    days_of_year: Optional[date] = Field(
        None, description="每年的某天，格式 YYYY/MM/DD")
    trigger_action: Optional[TriggerActionEnum] = Field(
        None, description="指定时间调度类型，按年year 按月month 按周week")


class Schedule(ScheduleBase):
    days_of_week: Optional[List[str]] = Field(
        None, description="每周的周几，可多选，格式 List[mon tue wed thu fri sat sun]")

    _preprocess_week = validator(
        "days_of_week", allow_reuse=True)(convert_list_to_str)

    class Config:
        orm_mode = True

    @root_validator
    def check_schedule_by_trigger_action(cls, schedule):
        CustomizeTriggerValidator.check_schedule_by_trigger_action(schedule)
        return schedule


class ScheduleQuery(ScheduleBase):
    days_of_week: Optional[str] = Field(
        None, description="每周的周几数据库1,2,3 返回前台格式 List[mon tue wed thu fri sat sun]")

    _proprocess_week = validator(
        "days_of_week", allow_reuse=True)(convert_str_to_list)

    class Config:
        orm_mode = True


class PolicyQuery(BaseModel):
    uuid: str = Field(None, description="唯一编码")
    name: str = Field(description="策略名称")
    action: Optional[PolicyActionEnum] = Field(
        None, description="策略动作（备份策略必填）：全量备份、日志备份、累积增量备份、差异增量备份、归档、复制")
    ext_parameters: Json = Field(None, description="扩展参数，不同类型应用各自扩展")
    retention: Optional[Retention] = Field(description="保留策略")
    schedule: ScheduleQuery = Field(description="策略调度信息")
    type: PolicyTypeEnum = Field(description="策略类型：备份策略、归档策略、复制策略")

    class Config:
        orm_mode = True

    @validator('ext_parameters', pre=True)
    def json_decode(cls, v):
        if isinstance(v, dict):
            try:
                return json.dumps(v)
            except ValueError:
                pass
        return v


class Policy(BaseModel):
    uuid: Optional[UUID] = Field(None, description="唯一编码")
    name: str = Field(description="策略名称", min_length=1, max_length=64,
                      regex="^[a-zA-Z_\\u4e00-\\u9fa5]{1}[\\u4e00-\\u9fa5\\w-]*$")
    action: Optional[PolicyActionEnum] = Field(
        None, description="策略动作：全量备份、日志备份、累积增量备份、差异增量备份、归档、复制")
    ext_parameters: BaseExtendParam = Field(None, description="扩展参数", )
    retention: Optional[Retention] = Field(description="保留策略")
    schedule: Schedule = Field(description="策略调度信息")
    type: PolicyTypeEnum = Field(description="策略类型：备份策略、归档策略、复制策略")

    @validator("uuid", pre=True)
    def validate_policy_id(cls, policy_uuid):
        if policy_uuid == "":
            policy_uuid = None
        return policy_uuid

    class Config:
        orm_mode = True
