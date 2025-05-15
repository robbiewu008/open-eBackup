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
from datetime import datetime
from typing import List

from app.archive.client.archive_client import ArchiveClient
from app.common import logger
from app.common.enums.sla_enum import CopyTypeEnum, TimeRangeMonthEnum, TimeRangeWeekEnum
from app.copy_catalog.service import copy_delete_workflow
from app.copy_catalog.models.tables_and_sessions import database, CopyArchiveMapTable, CopyTable

__all__ = ["CheckPolicy", "ImportCopy"]

from app.copy_catalog.service.copy_delete_workflow import CopyDeleteParam

from app.copy_catalog.service.curd import copy_query_service

log = logger.get_logger(__name__)


class CheckPolicy:
    def __init__(self, specified_scope):
        self.specified_scope = specified_scope
        self.weekday = self.format_week_day()

    @staticmethod
    def check_month(copy, month_scope, date_time):
        # 每月指定月条件，默认满足
        if not month_scope:
            return True

        order_by = CopyTable.display_timestamp.desc()
        if month_scope == TimeRangeMonthEnum.first:
            order_by = CopyTable.display_timestamp.asc()
        filters = (copy.get("resource_id"), copy.get("generated_by"), date_time.month, date_time.year)
        # 获取每月最新的或第一个副本
        resource_copy_id = copy_query_service.query_first_last_copy_id(*filters, order_by)
        # 不是最新或第一个副本
        if resource_copy_id != copy.get("uuid"):
            log.debug(f"copy({copy.get('uuid')}) is not last or first.")
            return False
        return True

    @staticmethod
    def format_week_day():
        week_day = dict()
        for key, value in enumerate(TimeRangeWeekEnum):
            week_day.update({value.value: key})
        return week_day

    @staticmethod
    def check_copy_chain(copy):
        # 查出这条链上的所有副本，找到待归档副本之前的副本
        dependency_copies = ArchiveClient.query_dependency_copy([copy.get('uuid', '')])
        dependency_copies = dependency_copies[0].get('dependencyCopyList')
        pre_copy = None
        for index, dp_copy in enumerate(dependency_copies):
            if dp_copy.get('uuid', '') == copy.get('uuid', '') and index != 0:
                pre_copy = dependency_copies[index - 1]
                break
            else:
                pre_copy = copy
        # 去归档副本表里面看看有没有归档过
        with database.session() as session:
            query_condition = [CopyArchiveMapTable.copy_id == pre_copy.get('uuid', '')]
            copies: List[str] = session.query(CopyArchiveMapTable.copy_id).filter(
                *query_condition).all()
        if copies:
            # 然后再看看归档的副本是否还存在，若都满足则可以归档此副本
            with database.session() as session:
                query_condition = [CopyTable.origin_backup_id == pre_copy.get('uuid', '')]
                archive_copy: List[str] = session.query(CopyTable).filter(
                    *query_condition).all()
            if archive_copy:
                return True
        else:
            return True
        return False

    def filter_copies(self, copy_list_archive: List):
        if not copy_list_archive:
            return copy_list_archive
        # 解析年、月、日
        year_scope, month_scope, week_scope = self.para_specified_scope()
        copy_filters = []
        for copy in copy_list_archive:
            # 副本生成时间
            date_time: datetime = datetime.fromisoformat(copy.get("generated_time"))
            generated_month = date_time.month

            # 副本生成时间是否满足年条件
            if year_scope and int(year_scope) != generated_month:
                log.debug("[ARCHIVE_TASK] Does not meet the year condition.")
                continue

            # 判断是否满足月条件
            if not self.check_month(copy, month_scope, date_time):
                log.debug("[ARCHIVE_TASK] Does not meet the month condition.")
                continue

            # 副本生成时间是否满足周条件
            if week_scope and self.weekday.get(week_scope, 0) != date_time.weekday():
                log.debug("[ARCHIVE_TASK] Does not meet the week condition.")
                continue
            copy_filters.append(copy)
        return copy_filters

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
            f"[ARCHIVE_TASK] Get a copy of the specified date policy: year:{year_scope}, month:{month_scope}, "
            f"week[{week_scope}]")
        return year_scope, month_scope, week_scope


class ImportCopy:
    def __init__(self, copy_info):
        self.copy_id = copy_info.get("uuid")
        self.user_id = copy_info.get("user_id")
        self.copy_delete_param = CopyDeleteParam(user_id=self.user_id, strict=False, create_job=True)

    def delete_copy(self):
        log.info(f"[ARCHIVE_TASK] Delete copy:{self.copy_id}.")
        copy_delete_workflow.request_delete_copy_by_id(self.copy_id, self.copy_delete_param)
