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
import time
from datetime import datetime

from app.backup.common.constant import ProtectionConstant
from app.backup.common.cron_utils import gen_cron_expression, get_cron_next_time
from app.common import logger
from app.common.enums.sla_enum import TriggerEnum

log = logger.get_logger(__name__)


def get_start_time_by_action(policy_list):
    """ 获取sla配置的备份策略的下次执行时间

    获取备份策略中，下次执行时间的时间戳

    Args:
        policy_list: 配置的sla备份策略

    Returns:
        每个策略对应的下次执行时间
    """
    start_time_dict = {}
    for policy in policy_list:
        schedule = policy.schedule
        start_time_timestamp = 0
        if schedule.trigger == TriggerEnum.interval:
            start_time_timestamp = create_interval_next_start_time(schedule)
        if schedule.trigger == TriggerEnum.customize_interval:
            start_time_timestamp = create_customize_next_start_time(schedule)
        start_time_dict[policy.action.value] = int(start_time_timestamp)
    return start_time_dict


def create_interval_next_start_time(schedule):
    start_time = schedule.start_time
    if isinstance(start_time, datetime):
        start_time = start_time.date()
    start_time = str(start_time) + " " + str(schedule.window_start)
    log.info(f"Get interval start time, start time={start_time}")
    return time.mktime(time.strptime(start_time, ProtectionConstant.DATE_TIME_FORMATTER))


def create_customize_next_start_time(schedule):
    cron_expr = gen_cron_expression(schedule)
    cron_next_time = get_cron_next_time(cron_expr)[0]
    log.info(f"Get customize start time, start time={cron_next_time}")
    return time.mktime(time.strptime(cron_next_time, ProtectionConstant.DATE_TIME_FORMATTER))
