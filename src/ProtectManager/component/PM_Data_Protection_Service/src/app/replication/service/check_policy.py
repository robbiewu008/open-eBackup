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
from collections import defaultdict
from datetime import datetime
from typing import List

from app.common import logger
from app.common.enums.sla_enum import CopyTypeEnum, TimeRangeWeekEnum, TimeRangeMonthEnum

log = logger.get_logger(__name__)


class CheckPolicy:
    def __init__(self, specified_scope):
        self.specified_scope = specified_scope
        self.weekday = self.format_week_day()

    @staticmethod
    def format_week_day():
        week_day = dict()
        for key, value in enumerate(TimeRangeWeekEnum):
            week_day.update({value.value: key})
        return week_day

    def filter_copies(self, copy_list: List):
        filters_copy = []
        if not copy_list:
            return filters_copy
        # 根据月份分组
        rows_by_date = defaultdict(list)
        for row in copy_list:
            date_time: datetime = datetime.fromisoformat(row.get("generated_time"))
            rows_by_date[date_time.month].append(row)

        # 解析年、月、日
        year_scope, month_scope, week_scope = self.para_specified_scope()
        for month, copies in rows_by_date.items():
            # 如果指定月条件，每月都至少有一个副本满足条件
            if month_scope:
                log.debug("[REPLICATION_TASK] has month scope")
                if month_scope == TimeRangeMonthEnum.first:
                    filters_copy.append(copies[0])
                    del copies[0]
                else:
                    filters_copy.append(copies.pop())
            for copy in copies:
                # 副本生成时间
                date_time: datetime = datetime.fromisoformat(copy.get("generated_time"))

                # 副本生成时间是否满足年条件
                if year_scope and int(year_scope) == month:
                    log.debug("[REPLICATION_TASK] meet the year condition.")
                    filters_copy.append(copy)
                    continue

                # 副本生成时间是否满足周条件
                if week_scope and self.weekday.get(week_scope, 0) == date_time.weekday():
                    log.debug("[REPLICATION_TASK] meet the week condition.")
                    filters_copy.append(copy)
                    continue
        return filters_copy

    def para_specified_scope(self):
        year_scope, month_scope, week_scope = None, None, None
        for scope in self.specified_scope:
            if scope.get("copy_type") == CopyTypeEnum.year:
                year_scope = scope.get("generate_time_range")
            elif scope.get("copy_type") == CopyTypeEnum.month:
                month_scope = scope.get("generate_time_range")
            else:
                week_scope = scope.get("generate_time_range")
        log.info(
            f"[REPLICATION_TASK] Get a copy of the specified date policy: year:{year_scope}, month:{month_scope}, "
            f"week[{week_scope}]")
        return year_scope, month_scope, week_scope