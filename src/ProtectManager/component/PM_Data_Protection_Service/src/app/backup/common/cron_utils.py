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
import time
from http import HTTPStatus
from urllib.parse import quote

from app.backup.schemas.policy import Schedule
from app.common import logger
from app.common.clients.client_util import SystemBaseHttpsClient
from app.common.enums.sla_enum import TriggerEnum
from app.common.exter_attack import exter_attack

LOGGER = logger.get_logger(__name__)


def gen_cron_expression(schedule: Schedule):
    """ 获取cron表达式

    将sla的备份策略中schedule的指定时间转换为cron表达式

    Args:
        schedule: sla的备份策略中的schedule

    Returns:
        生成的cron表达式字符串，可以通过该cron表达式获取执行时间
        example:

        指定每月1日和30日的22:30:00执行备份，则对应的cron表达式为：
        "0 30 22 1,30 * ?"
    """
    error_result = ""
    if schedule.trigger == TriggerEnum.interval:
        return error_result
    window_start = schedule.window_start

    cron_expression = ""
    cron_expression += (str(window_start.second) + " ")
    cron_expression += (str(window_start.minute) + " ")
    cron_expression += (str(window_start.hour) + " ")

    if schedule.trigger_action == schedule.trigger_action.week:
        cron_expression += "? * "
        cron_expression += schedule.days_of_week
    if schedule.trigger_action == schedule.trigger_action.month:
        if "32" in schedule.days_of_month:
            cron_expression += "L"
        else:
            cron_expression += schedule.days_of_month
        cron_expression += " * ?"
    if schedule.trigger_action == schedule.trigger_action.year:
        cron_expression += str(schedule.days_of_year.day) + " "
        cron_expression += str(schedule.days_of_year.month) + " ? *"

    return cron_expression


@exter_attack
def get_cron_next_time(cron_expr: str):
    """获取下次执行时间

    Args:
        cron_expr: cron表达式

    Returns:
        该cron表达式基于当前时间，解析到的下次执行时间

    Raises：
        EmeiStorBizException：rest调用失败则抛出异常

    通过SystemBase中Schedule模块提供的服务，传入cron表达式，获得基于当前时间的下次执行时间
    """
    url = f"/v1/internal/schedules/crons/{quote(cron_expr)}/next-time"
    LOGGER.info(f'invoke api to get schedule cron next time, request url:{url}')
    start = time.time()
    response = SystemBaseHttpsClient().request("GET", url, build_exception=True)
    end = time.time()
    if response.status == HTTPStatus.OK:
        LOGGER.info(f'invoke api to get schedule cron next time, next time:{response.data}, cost:{end - start}s')
        return json.loads(response.data)
    else:
        LOGGER.error(f"invoke api to get schedule cron next time, url:{url}, cost:{end - start}s")
