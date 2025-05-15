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
import calendar
import datetime
from enum import Enum, IntEnum
from typing import Dict, Callable


class WeekDaysEnum(Enum):
    sun = "1"
    mon = "2"
    tue = "3"
    wed = "4"
    thu = "5"
    fri = "6"
    sat = "7"

    def get_value(self):
        return WeekDaysEnum[self].value

    def get_key(self):
        return WeekDaysEnum(self).name


class MonthDayEnum(str, Enum):
    last_day_of_each_month = "32"


class PolicyTypeEnum(str, Enum):
    backup = "backup"
    archiving = "archiving"
    replication = "replication"


class PolicyActionEnum(str, Enum):
    full = "full"
    log = "log"
    cumulative_increment = "cumulative_increment"
    difference_increment = "difference_increment"
    replication = "replication"
    replication_log = "replication_log"
    archiving = "archiving"
    permanent_increment = "permanent_increment"
    snapshot = "snapshot"


class TimeRangeYearEnum(str, Enum):
    jan = "1"
    feb = "2"
    mar = "3"
    ari = "4"
    may = "5"
    jun = "6"
    jul = "7"
    aut = "8"
    sep = "9"
    oct = "10"
    nov = "11"
    dec = "12"


class TimeRangeMonthEnum(str, Enum):
    last = "last"
    first = "first"


class TimeRangeWeekEnum(str, Enum):
    mon = "mon"
    tue = "tue"
    wed = "wed"
    thu = "thu"
    fri = "fri"
    sat = "sat"
    sun = "sun"


class ArchiveScope(str, Enum):
    latest = "latest"
    all_no_archiving = "all_no_archiving"


class ArchiveTypeEnum(IntEnum):
    all_copy = 1  # 归档所有副本
    specified_copy = 2  # 指定时间副本


class CopyTypeEnum(str, Enum):
    year = "year"  # 年类型
    month = "month"  # 月类型
    week = "week"  # 周类型


class TriggerEnum(IntEnum):
    interval = 1  # 周期执行
    backup_complete = 2  # 备份完立即执行
    after_backup_complete = 3  # 3-备份完指定时间执行
    customize_interval = 4  # 指定时间周期执行


class TriggerActionEnum(str, Enum):
    year = "year"  # 按照年
    month = "month"  # 按照月
    week = "week"  # 按照周


class RetentionTypeEnum(IntEnum):
    permanent = 1  # 永久保留
    temporary = 2  # 按时间保留
    QUANTITY = 3  # 按数量保留


class WormValidityTypeEnum(IntEnum):
    worm_not_open = 0  # worm开启
    copy_retention_time_consistent = 1  # 同副本保留时间一致
    custom_retention_time = 2  # 自定义worm过期时间


class WindowTimeUnit(str, Enum):
    minutes = "m"
    hours = "h"


class RetentionTimeUnit(str, Enum):
    days = "d"
    weeks = "w"
    months = "MO"
    years = "y"


class DeleteTimeUnit(str, Enum):
    hours = "h"
    days = "d"
    weeks = "w"
    months = "MO"
    years = "y"


class BackupTypeEnum(IntEnum):
    full = 1  # 全量备份
    cumulative_increment = 2  # 增量备份
    difference_increment = 3  # 差异备份
    log = 4  # 事务日志备份
    permanent_increment = 5  # 永久增量备份
    snapshot = 6  # 快照备份


class ReplicationTypeEnum(IntEnum):
    ALL_COPY = 1  # 复制所有副本
    SPECIFIED_COPY = 2  # 指定时间副本


class ReplicationModeEnum(IntEnum):
    EXTRA = 1
    INTRA = 2


class ReplicationStorageTypeEnum(IntEnum):
    BACKUP_STORAGE_UNIT_GROUP = 1  # 备份存储单元组
    BACKUP_STORAGE_UNIT = 2  # 备份存储单元


def add_time(time: datetime.datetime, delta: int, unit: RetentionTimeUnit):
    mapping: Dict[RetentionTimeUnit, Callable[[datetime.datetime, int], datetime.datetime]] = {
        RetentionTimeUnit.days: __add_day,
        RetentionTimeUnit.weeks: __add_week,
        RetentionTimeUnit.months: __add_month,
        RetentionTimeUnit.years: __add_year,
    }
    func = mapping[unit]
    return func(time, delta)


def __add_day(time: datetime.datetime, delta: int):
    return time + datetime.timedelta(days=delta)


def __add_week(time: datetime.datetime, delta: int):
    return time + datetime.timedelta(weeks=delta)


def __add_month(time: datetime.datetime, delta: int):
    month = time.month - 1 + delta
    year = time.year + month // 12
    month = month % 12 + 1
    day = min(time.day, calendar.monthrange(year, month)[1])
    return time.replace(year=year, month=month, day=day)


def __add_year(time: datetime.datetime, delta: int):
    year = time.year + delta
    day = min(time.day, calendar.monthrange(year, time.month)[1])
    return time.replace(year=year, day=day)


class BackupTimeUnit(str, Enum):
    minutes = "m"
    hours = "h"
    days = "d"
    weeks = "w"
    months = "MO"
    years = "y"


class IntervalUnit(str, Enum):
    minutes = "m"
    hours = "h"
    days = "d"
    weeks = "w"


class SlaType(IntEnum):
    # 备份
    backup = 1
    # 容灾
    disaster_recovery = 2


# 单个策略业务校验触发条件
class SinglePolicyBusinessValidatorRule(str, Enum):
    validate_business_backup = "validate_business_backup"
    validate_business_archive = "validate_business_archive"
    validate_business_replication = "validate_business_replication"


# 多个策略业务校验触发条件
class MultiPoliciesBusinessValidatorRule(str, Enum):
    validate_business_policies = "validate_business_policies"


# 多个策略规格校验触发条件
class MultiPoliciesSpecificationValidatorRule(str, Enum):
    validate_specification_backup = "validate_specification_backup"
    validate_specification_archive = "validate_specification_archive"
    validate_specification_replication = "validate_specification_replication"
    validate_specification_sla = "validate_specification_sla"


time_interval_switcher = {
    BackupTimeUnit.minutes.value: lambda x: x * 60,
    BackupTimeUnit.hours.value: lambda x: x * 60 * 60,
    BackupTimeUnit.days.value: lambda x: x * 24 * 60 * 60,
    BackupTimeUnit.weeks.value: lambda x: x * 7 * 24 * 60 * 60,
    RetentionTimeUnit.months.value: lambda x: x * 4 * 7 * 24 * 60 * 60,
    RetentionTimeUnit.years.value: lambda x: x * 12 * 4 * 7 * 24 * 60 * 60
}


class HbaseBackupMode(str, Enum):
    Snapshot = "Snapshot"
    WAL = "WAL"


class FileSystemShareType(str, Enum):
    NFS = "NFS"
    CIFS = "CIFS"


class RepositoryProtocolEnum(IntEnum):
    CIFS = 0
    NFS = 1
    S3 = 2
    BLOCK = 3
    LOCAL_DIR = 4
    NATIVE_NFS = 5
    NATIVE_CIFS = 6
    TAPE = 7


retention_time_limits = {
    RetentionTimeUnit.days: 365,
    RetentionTimeUnit.weeks: 54,
    RetentionTimeUnit.months: 24,
    RetentionTimeUnit.years: 10,
}
